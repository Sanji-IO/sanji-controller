/*
 * ##########################
 * HEADER FILES
 * ##########################
 */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#include <winsock2.h>
#define snprintf sprintf_s
#endif

#include <mosquitto.h>
#include <jansson.h>
#include "sanji_controller.h"
#include "list.h"
#include "debug.h"
#include "resource.h"
#include "component.h"
#include "session.h"


/*
 * ##########################
 * GLOBAL VARIABLES
 * ##########################
 */
static int sanji_run = 1;

struct resource *sanji_resource = NULL;
struct component *sanji_component = NULL;
struct session *sanji_session = NULL;

/*
 * ##########################
 * MISC FUNCTIONS
 * ##########################
 */
int rand_generator(int mode)
{
#ifndef WIN32
	static int num = 0;
	unsigned int seed;
	FILE *urandom = NULL;

	switch (mode) {
	case 0:
		return ++num;
		break;
	case 1:
	default:
		urandom = fopen("/dev/urandom", "r");
		fread(&seed, sizeof(int), 1, urandom);
		fclose(urandom);
		srand(seed);
		return rand();
	}
#else
#endif
}

/*
 * ##########################
 * SANJI FUNCTIONS
 * ##########################
 */
void sanji_userdata_free(struct sanji_userdata *ud)
{
	if (ud) {
		if (ud->topics) {
			free(ud->topics);
		}
		free(ud);
	}
}

void sanji_print_usage(void)
{
	int major, minor, revision;

	mosquitto_lib_version(&major, &minor, &revision);
	printf("mosquitto_sub is a simple mqtt client that will subscribe to a single topic and print all messages it receives.\n");
	printf("mosquitto_sub version %s running on libmosquitto %d.%d.%d.\n\n", SANJI_VERSION, major, minor, revision);
	printf("Usage: mosquitto_sub [-c] [-h host] [-k keepalive] [-p port] [-q qos] [-R] [-v] -t topic ...\n");
	printf("                     [-i id] [-I id_prefix]\n");
	printf("                     [-d] [--quiet]\n");
	printf("                     [-u username [-P password]]\n");
	printf("                     [--will-topic [--will-payload payload] [--will-qos qos] [--will-retain]]\n");
	printf("       mosquitto_sub --help\n\n");
	printf(" -c : disable 'clean session' (store subscription and pending messages when client disconnects).\n");
	printf(" -d : enable debug messages.\n");
	printf(" -h : mqtt host to connect to. Defaults to localhost.\n");
	printf(" -i : id to use for this client. Defaults to mosquitto_sub_ appended with the process id.\n");
	printf(" -I : define the client id as id_prefix appended with the process id. Useful for when the\n");
	printf("      broker is using the clientid_prefixes option.\n");
	printf(" -k : keep alive in seconds for this client. Defaults to 60.\n");
	printf(" -p : network port to connect to. Defaults to 1883.\n");
	printf(" -q : quality of service level to use for the subscription. Defaults to 0.\n");
	printf(" -R : do not print stale messages (those with retain set).\n");
	printf(" -t : mqtt topic to subscribe to. May be repeated multiple times.\n");
	printf(" -u : provide a username (requires MQTT 3.1 broker)\n");
	printf(" -v : print published messages verbosely.\n");
	printf(" -P : provide a password (requires MQTT 3.1 broker)\n");
	printf(" --help : display this message.\n");
	printf(" --quiet : don't print error messages.\n");
	printf(" --will-payload : payload for the client Will, which is sent by the broker in case of\n");
	printf("                  unexpected disconnection. If not given and will-topic is set, a zero\n");
	printf("                  length message will be sent.\n");
	printf(" --will-qos : QoS level for the client Will.\n");
	printf(" --will-retain : if given, make the client Will retained.\n");
	printf(" --will-topic : the topic on which to publish the client Will.\n");
	printf("\nSee http://mosquitto.org/ for more information.\n\n");
}

void sanji_signal_handler(int s)
{
	fprintf(stderr, "SANJI: signal_handler: get stop signal.\n");
	sanji_run = 0;
}

int sanji_register_response_error(struct mosquitto *mosq, void *obj, json_t *root, char *tunnel, int status_code)
{
	/* reponse context */
	struct sanji_userdata *ud = NULL;
	json_t *root_response = NULL;
	char *context;
	int context_len;

	if (!mosq || !obj || !root || !tunnel) return 1;

	ud = (struct sanji_userdata *)obj;

	/* create json reponse context */
	root_response = json_object();
	if (!root_response) {
		DEBUG_PRINT("Error: out of memory");
		return 1;
	}

	/* add 'code' */
	json_object_set_new(root, "code", json_integer(status_code));
	context = json_dumps(root, JSON_INDENT(4));
	if (!context) {
		DEBUG_PRINT("Error: out of memory");
		json_decref(root_response);
		return 1;
	}
	context_len = strlen(context);

	/* publish a response */
	mosquitto_publish(mosq, &ud->mid_sent, tunnel, context_len, context, ud->qos_sent, ud->retain_sent);

	free(context);
	json_decref(root_response);

	return 0;
}

int sanji_register_response(struct mosquitto *mosq, void *obj, json_t *root, json_t *data, char *tunnel, char *name, int method, int status_code)
{
	/* reponse context */
	struct component *curr = NULL;
	struct sanji_userdata *ud = NULL;
	json_t *root_response = NULL;
	char *context = NULL;
	int context_len;

	if (!mosq || !obj || !root || !data || !tunnel || !name) return 1;

	ud = (struct sanji_userdata *)obj;

	/* create json reponse context */
	root_response = json_object();
	if (!root_response) {
		DEBUG_PRINT("Error: out of memory");
		return 1;
	}

	if (method == RESOURCE_METHOD_CREATE) {
		/* update 'tunnel' */
		if (status_code == SESSION_CODE_OK) {
			/* ENHANCE: need encapsulate */
			list_for_each_entry(curr, &sanji_component->list, list) {
				if (!strcmp(curr->name, name)) {
					json_object_set_new(data, "tunnel", json_string(curr->tunnel));
					break;
				}
			}
		}
	} else {
		/* get 'tunnel' from component list */
		/* ENHANCE: need encapsulate */
		list_for_each_entry(curr, &sanji_component->list, list) {
			if (!strcmp(curr->name, name)) {
				memset(tunnel, '\0', COMPONENT_TUNNEL_LEN);
				strncpy(tunnel, curr->tunnel, COMPONENT_TUNNEL_LEN);
				break;
			}

		}
	}

	/* add 'code' */
	json_object_set_new(root, "code", json_integer(status_code));
	context = json_dumps(root, JSON_INDENT(4));
	if (!context) {
		DEBUG_PRINT("Error: out of memory");
		json_decref(root_response);
		return 1;
	}
	context_len = strlen(context);


	/* publish a response */
	mosquitto_publish(mosq, &ud->mid_sent, tunnel, context_len, context, ud->qos_sent, ud->retain_sent);

	free(context);
	json_decref(root_response);

	return 0;
}


int sanji_register_create(char *name, char *description, char *role, char *hook, unsigned int hook_count, int ttl, char *data_resource, unsigned int data_resource_count)
{
	struct component *curr = NULL;
	int is_registered = 0;
	int is_not_unique = 0;
	int is_locked = 0;
	int random_num;
	char tunnel[COMPONENT_TUNNEL_LEN];
	int i;

	if (!name || !description || !role || !data_resource) return SESSION_CODE_INTERNAL_SERVER_ERROR;

	/* check whether if one of resources is locked */
	for (i = 0; i < data_resource_count; i++) {
		is_locked += resource_is_locked(sanji_resource, data_resource + i * COMPONENT_NAME_LEN);
	}
	if (is_locked) {
		DEBUG_PRINT("Error: resoures is locked.");
		return SESSION_CODE_LOCKED;
	}

	/*
	 * create component object
	 */
	/* check whether already been registered */
	is_registered = component_is_registered(sanji_component, name);
	if (is_registered == -1) {
		return SESSION_CODE_INTERNAL_SERVER_ERROR;
	} else if (is_registered) {
		DEBUG_PRINT("Error: component has already been registered.");
		return SESSION_CODE_FORBIDDEN;
	}

	/* init a unique random tunnel */
	do {
		random_num = rand_generator(SANJI_RAND_MODE_SEQ);
		/* ENHANCE: need encapsulate */
		list_for_each_entry(curr, &sanji_component->list, list) {
			is_not_unique = 1;
			if (random_num != atoi(curr->tunnel)) is_not_unique = 0;
		}
	} while (is_not_unique);

	memset(tunnel, '\0', COMPONENT_TUNNEL_LEN);
	sprintf(tunnel, "%d", random_num);

	if (component_add_node(sanji_component, name, description, tunnel, role, hook, hook_count, ttl, 0)) {
		DEBUG_PRINT("Error: out of memory");
		return SESSION_CODE_SERVICE_UNAVAILABLE;
	}

#ifdef DEBUG
	component_display(sanji_component);
#endif

	/*
	 * create resource object
	 */
	for (i = 0; i < data_resource_count; i++) {
		resource_append_component_by_name(sanji_resource, data_resource + i * COMPONENT_NAME_LEN, name);
	}

#ifdef DEBUG
	resource_display(sanji_resource);
#endif

	return SESSION_CODE_OK;
}

int sanji_register_delete(char *name)
{
	int is_locked = 0;
	int is_registered = 0;

	if (!name) return SESSION_CODE_INTERNAL_SERVER_ERROR;

	/* check whether if one of resources/components is locked */
	is_locked += component_is_locked(sanji_component, name);
	is_locked += resource_is_any_locked_by_component(sanji_resource, name);
	if (is_locked) {
		DEBUG_PRINT("Error: resoures is locked.");
		return SESSION_CODE_LOCKED;
	}

	/*
	 * delete component object
	 */
	/* check whether already been deleted */
	is_registered = component_is_registered(sanji_component, name);
	if (is_registered == -1) {
		return SESSION_CODE_INTERNAL_SERVER_ERROR;
	} else if (!is_registered) {
		return SESSION_CODE_OK;
	}

	if (component_delete_first_name(sanji_component, name)) {
		DEBUG_PRINT("Error: out of memory");
		return SESSION_CODE_INTERNAL_SERVER_ERROR;
	}

#ifdef DEBUG
	component_display(sanji_component);
#endif

	/*
	 * delete resource object
	 */
	resource_remove_all_component_by_name(sanji_resource, name);

#ifdef DEBUG
	resource_display(sanji_resource);
#endif

	return SESSION_CODE_OK;

	return 0;
}

int sanji_register_procedure(
		struct mosquitto *mosq,
		void *obj,
		json_t *root,
		int id,
		int method,
		char *resource,
		int code,
		json_t *data)
{
	/* json context only for register data */
	char name[COMPONENT_NAME_LEN];
	char description[COMPONENT_DESCRIPTION_LEN];
	char _tunnel[COMPONENT_TUNNEL_LEN];
	char role[COMPONENT_ROLE_LEN];
	json_t *_hook = NULL;			// hook array
	char *hook = NULL;
	unsigned int hook_count = 0;
	int ttl;
	json_t *_data_resource = NULL;
	char *data_resource = NULL;
	unsigned int data_resource_count = 0;

	/* other var */
	json_t *tmp = NULL;
	int status_code;
	int i;

	/* verify and prepare attribute value to register */
	if (!data && method == RESOURCE_METHOD_CREATE) {
		DEBUG_PRINT("Error: wrong sanji packet, no 'data'.");
		return 1;
	}

	/*
	 * Note:
	 * After acquire 'tunnel' we can response error back.
	 * So we acquire 'tunnel' first.
	 */
	memset(_tunnel, '\0', COMPONENT_TUNNEL_LEN);
	if (method == RESOURCE_METHOD_CREATE) {
		tmp = json_object_get(data, "tunnel");
		if (tmp && json_is_string(tmp)) {
			strncpy(_tunnel, json_string_value(tmp), COMPONENT_TUNNEL_LEN);
			_tunnel[COMPONENT_TUNNEL_LEN - 1] = '\0';
			DEBUG_PRINT("tunnel: '%s'", _tunnel);
		} else {
			DEBUG_PRINT("Error: wrong sanji packet, no 'tunnel'.");
			return 1;
		}
	}

	/*
	 * acquire 'name' from identifier,
	 * since we use 'name' attribute to be the idetifier of registration resource
	 */
	memset(name, '\0', COMPONENT_NAME_LEN);
	if (strlen(resource) > SANJI_REGISTER_TOPIC_LEN) {
		if (!strchr(&resource[SANJI_REGISTER_TOPIC_LEN + 1], '/')) {
			strncpy(name, &resource[SANJI_REGISTER_TOPIC_LEN + 1], COMPONENT_NAME_LEN);
			name[COMPONENT_NAME_LEN - 1] = '\0';
			DEBUG_PRINT("name: '%s'", name);
		} else {
			DEBUG_PRINT("Error: wrong register topic");
			sanji_register_response_error(mosq, obj, root, _tunnel, SESSION_CODE_NOT_FOUND);
			return 1;
		}
	} else {
		/* use 'name' in attribute */
		tmp = json_object_get(data, "name");
		if (tmp && json_is_string(tmp)) {
			strncpy(name, json_string_value(tmp), COMPONENT_NAME_LEN);
			name[COMPONENT_NAME_LEN - 1] = '\0';
			DEBUG_PRINT("name: '%s'", name);
		} else {
			DEBUG_PRINT("Error: wrong sanji packet, no 'name'.");
			sanji_register_response_error(mosq, obj, root, _tunnel, SESSION_CODE_BAD_REQUEST);
			return 1;
		}
	}

	/* CRUD procedure */
	switch (method) {

	case RESOURCE_METHOD_CREATE:
		DEBUG_PRINT("create method");

		/* verify and prepare attribute value to register */
		memset(description, '\0', COMPONENT_DESCRIPTION_LEN);
		tmp = json_object_get(data, "description");
		if (tmp && json_is_string(tmp)) {
			strncpy(description, json_string_value(tmp), COMPONENT_DESCRIPTION_LEN);
			description[COMPONENT_DESCRIPTION_LEN - 1] = '\0';
			DEBUG_PRINT("description: '%s'", description);
		} else {
			DEBUG_PRINT("Error: wrong sanji packet, no 'description'.");
			sanji_register_response_error(mosq, obj, root, _tunnel, SESSION_CODE_BAD_REQUEST);
			return 1;
		}

		memset(role, '\0', COMPONENT_ROLE_LEN);
		tmp = json_object_get(data, "role");
		if (tmp && json_is_string(tmp)) {
			strncpy(role, json_string_value(tmp), COMPONENT_ROLE_LEN);
			role[COMPONENT_ROLE_LEN - 1] = '\0';
			DEBUG_PRINT("role: '%s'", role);
		} else {
			DEBUG_PRINT("Error: wrong sanji packet, no 'role'.");
			sanji_register_response_error(mosq, obj, root, _tunnel, SESSION_CODE_BAD_REQUEST);
			return 1;
		}

		/*
		 * Note:
		 * After acquire 'hook', we must free hook before return.
		 */
		_hook = json_object_get(data, "hook");
		if (_hook && json_is_array(_hook)) {
			hook_count = json_array_size(_hook);
			hook = (char *)malloc(hook_count * sizeof(char) * COMPONENT_NAME_LEN);
			if (!hook) {
				DEBUG_PRINT("Error: out of memory");
				return 1;
			}
			for (i = 0; i < hook_count; i++) {
				tmp = json_array_get(_hook, i);
				if (tmp && json_is_string(tmp)) {
					strncpy(hook + (i * COMPONENT_NAME_LEN), json_string_value(tmp), COMPONENT_NAME_LEN);
					hook[(i + 1) * COMPONENT_NAME_LEN - 1] = '\0';
				}
			}
		}

		tmp = json_object_get(data, "ttl");
		if (tmp && json_is_number(tmp)) {
			ttl = json_number_value(tmp);
			DEBUG_PRINT("ttl: '%d'", ttl);
		} else {
			DEBUG_PRINT("Error: wrong sanji packet, no 'ttl'.");
			sanji_register_response_error(mosq, obj, root, _tunnel, SESSION_CODE_BAD_REQUEST);
			if (hook) free(hook);
			return 1;
		}

		/*
		 * Note:
		 * After acquire 'data_resource', we must free data_resource before return.
		 */
		_data_resource = json_object_get(data, "resource");
		if (_data_resource && json_is_array(_data_resource)) {
			data_resource_count = json_array_size(_data_resource);
			data_resource = (char *)malloc(data_resource_count * sizeof(char) * RESOURCE_NAME_LEN);
			if (!data_resource) {
				DEBUG_PRINT("Error: out of memory");
				if (hook) free(hook);
				return 1;
			}
			for (i = 0; i < data_resource_count; i++) {
				tmp = json_array_get(_data_resource, i);
				if (tmp && json_is_string(tmp)) {
					strncpy(data_resource + (i * COMPONENT_NAME_LEN), json_string_value(tmp), COMPONENT_NAME_LEN);
					data_resource[(i + 1) * COMPONENT_NAME_LEN - 1] = '\0';
				}
			}
		} else {
			DEBUG_PRINT("Error: wrong sanji packet, no 'resource'.");
			sanji_register_response_error(mosq, obj, root, _tunnel, SESSION_CODE_BAD_REQUEST);
			if (hook) free(hook);
			return 1;
		}

		/* create register */
		status_code = sanji_register_create(name, description, role, hook, hook_count, ttl, data_resource, data_resource_count);
		sanji_register_response(mosq, obj, root, data, _tunnel, name, method, status_code);

		break;

	case RESOURCE_METHOD_READ:
		DEBUG_PRINT("read method");
		sanji_register_response_error(mosq, obj, root, _tunnel, SESSION_CODE_METHOD_NOT_ALLOWED);
		break;

	case RESOURCE_METHOD_UPDATE:
		DEBUG_PRINT("update method");
		sanji_register_response_error(mosq, obj, root, _tunnel, SESSION_CODE_METHOD_NOT_ALLOWED);
		break;

	case RESOURCE_METHOD_DELETE:
		DEBUG_PRINT("delete method");
		/* delete register */
		status_code = sanji_register_delete(name);
		sanji_register_response(mosq, obj, root, data, _tunnel, name, method, status_code);
		break;

	default:
		DEBUG_PRINT("unknown method");
		sanji_register_response_error(mosq, obj, root, _tunnel, SESSION_CODE_BAD_REQUEST);
		break;
	}

	/* clear */
	if (hook) free(hook);
	if (data_resource) free(data_resource);

	DEBUG_PRINT("finish register procedure");

	return 0;
}

void sanji_routing_response(int i)
{
}

int sanji_routing_procedure(
		struct mosquitto *mosq,
		void *obj,
		json_t *root,
		int id,
		int method,
		char *topic,
		int code,
		json_t *data)
{

	/* request */
	struct resource *resource = NULL;
	char truncated_topic[COMPONENT_NAME_LEN];
	char *p = NULL;
	int is_inflight = 0;
	int is_locked = 0;
	struct component *component = NULL;
	char *subscribed_component = NULL;
	unsigned int subscribed_count;
	int is_model = 0;
	int is_view = 0;
	struct model_chain *model_chain = NULL;
	unsigned int model_chain_count = 0;
	char *hook_component = NULL;
	char *_hook_component = NULL;
	unsigned int hook_count = 0;
	unsigned int last_hook_count = 0;
	char *view_chain = NULL;
	int view_chain_count = 0;
	struct session *session_new = NULL;
	json_t *result_chain = NULL;
	int ret;

	/* response */
	struct session *session = NULL;

	/* other variable */
	struct model_chain *model_chain_tmp = NULL;
	char *models_tmp = NULL;
	int *ttls_tmp = NULL;
	char *hook_tmp = NULL;
	char *view_chain_tmp = NULL;
	int is_first_time = 1;
	int i;

	/* verify arguments */
	if (!mosq || !obj || !root || !topic) {
		DEBUG_PRINT("Error: missing necessary arguments.");
		return 1;
	}

	/* branch to request/response procedure */
	if (code < 0) {
		/* request procedure */
		DEBUG_PRINT("Request routing procedure");

		/* is session inflight */
		is_inflight = session_is_inflight(sanji_session, id);
		if (is_inflight < 0) {
			DEBUG_PRINT("Error: sanji session crash.");
			/* TODO */
			sanji_routing_response(SESSION_CODE_INTERNAL_SERVER_ERROR);
			return 1;
		} else if (is_inflight) {
			DEBUG_PRINT("Session is inflight.");
			/* TODO */
			sanji_routing_response(SESSION_CODE_FORBIDDEN);
			return 1;
		}

		/* lookup resource */
		resource = resource_lookup_node_by_name(sanji_resource, topic);
		if (!resource) {
			/* truncate the last fragment */
			memset(truncated_topic, '\0', COMPONENT_NAME_LEN);
			strncpy(truncated_topic, topic, COMPONENT_NAME_LEN);
			p = strrchr(truncated_topic, '/');
			if (p) {
				*p = '\0';
				resource = resource_lookup_node_by_name(sanji_resource, truncated_topic);
			}
			if (!resource) {
				DEBUG_PRINT("Resource didn't be registered.");
				/* TODO */
				sanji_routing_response(SESSION_CODE_NOT_FOUND);
				return 1;
			}
		}

		/* check resource locked */
		is_locked += resource_node_is_locked(resource);
		if (is_locked) {
			DEBUG_PRINT("Resource is locked.");
			/* TODO */
			sanji_routing_response(SESSION_CODE_LOCKED);
			return 1;
		}

		/* find model chain and view chain */
		subscribed_component = resource_get_subscribed_component(resource);
		subscribed_count = resource_get_subscribed_count(resource);
		if (!subscribed_component || !subscribed_count) {
			DEBUG_PRINT("Resource not implemented.");
			/* TODO */
			sanji_routing_response(SESSION_CODE_NOT_IMPLEMENTED);
			return 1;
		}
		do {
			if (!is_first_time) {
				subscribed_component = hook_component;
				subscribed_count = hook_count;
				hook_component = NULL;
				hook_count = 0;
			}
#ifdef DEBUG
			DEBUG_PRINT("subscribed_count(%d)", subscribed_count);
			for (i = 0; i < subscribed_count; i++) {
				DEBUG_PRINT("subscribed_component(%s)", subscribed_component + i * COMPONENT_NAME_LEN);
			}
#endif

			/* reallocate model chain and view chain */
			model_chain_count++;
			model_chain_tmp = (struct model_chain *)realloc(model_chain, model_chain_count * sizeof(struct model_chain));
			if (!model_chain_tmp) {
				DEBUG_PRINT("Error: out of memory");
				/* TODO */
				sanji_routing_response(SESSION_CODE_SERVICE_UNAVAILABLE);
				if (model_chain) session_free_model_chain(model_chain, model_chain_count);
				if (!is_first_time && subscribed_component) free(subscribed_component);
				return 1;
			}
			model_chain = model_chain_tmp;

			/* reuse model_chain_tmp as model_chain index */
			model_chain_tmp = &model_chain[model_chain_count - 1];
			memset(model_chain_tmp, 0, sizeof(struct model_chain));

			/* reallocate models/views */
			for (i = 0; i < subscribed_count; i++) {
				/* lookup component */
				component = component_lookup_node_by_name(sanji_component, subscribed_component + i * COMPONENT_NAME_LEN);
				if (!component) {
					DEBUG_PRINT("Component didn't be registered.");
					continue;
				}

				/* lookup model */
				is_model = component_node_is_given_role(component, "model");
				if (is_model) {
					/* check model locked */
					is_locked = component_node_is_locked(component);
					if (is_locked) {
						DEBUG_PRINT("Error: model is busy.");
						/* TODO */
						sanji_routing_response(SESSION_CODE_LOCKED);
						if (model_chain) session_free_model_chain(model_chain, model_chain_count);
						if (!is_first_time && subscribed_component) free(subscribed_component);
						if (hook_component) free(hook_component);
						return 1;
					}

					/* increase count */
					model_chain_tmp->count++;

					/* reallocate models here */
					models_tmp = (char *)realloc(model_chain_tmp->models, model_chain_tmp->count * COMPONENT_NAME_LEN);
					if (!models_tmp) {
						DEBUG_PRINT("Error: out of memory");
						/* TODO */
						sanji_routing_response(SESSION_CODE_SERVICE_UNAVAILABLE);
						if (model_chain) session_free_model_chain(model_chain, model_chain_count);
						if (!is_first_time && subscribed_component) free(subscribed_component);
						if (hook_component) free(hook_component);
						return 1;
					}
					model_chain_tmp->models = models_tmp;
					memset(model_chain_tmp->models + (model_chain_tmp->count - 1) * COMPONENT_NAME_LEN, '\0', COMPONENT_NAME_LEN);
					memcpy(model_chain_tmp->models + (model_chain_tmp->count - 1) * COMPONENT_NAME_LEN, component->name, COMPONENT_NAME_LEN);

					/* reallocate ttls here */
					ttls_tmp = (int *)realloc(model_chain_tmp->ttls, model_chain_tmp->count * sizeof(int));
					if (!ttls_tmp) {
						DEBUG_PRINT("Error: out of memory");
						/* TODO */
						sanji_routing_response(SESSION_CODE_SERVICE_UNAVAILABLE);
						if (model_chain) session_free_model_chain(model_chain, model_chain_count);
						if (!is_first_time && subscribed_component) free(subscribed_component);
						if (hook_component) free(hook_component);
						return 1;
					}
					model_chain_tmp->ttls = ttls_tmp;
					model_chain_tmp->ttls[model_chain_tmp->count - 1] = component->ttl;

					/* update hook models and hook count for write-like method */
					if (resource_is_write_like_method(method)) {
						last_hook_count = hook_count;
						_hook_component = component_get_names_by_hook(sanji_component, component->name, &hook_count);

						if (_hook_component) {
							hook_tmp = (char *)realloc(hook_component, hook_count * COMPONENT_NAME_LEN);
							if (!hook_tmp) {
								DEBUG_PRINT("Error: out of memory");
								/* TODO */
								sanji_routing_response(SESSION_CODE_SERVICE_UNAVAILABLE);
								if (model_chain) session_free_model_chain(model_chain, model_chain_count);
								if (_hook_component) free(_hook_component);
								if (hook_component) free(hook_component);
								return 1;
							}
							hook_component = hook_tmp;
							memset(hook_component + last_hook_count * COMPONENT_NAME_LEN, '\0', hook_count * COMPONENT_NAME_LEN);
							memcpy(hook_component + last_hook_count * COMPONENT_NAME_LEN, _hook_component, hook_count * COMPONENT_NAME_LEN);
							free(_hook_component);
						}
						hook_count += last_hook_count;
					}
				}

				/* lookup view */
				is_view = component_node_is_given_role(component, "view");
				if (is_view) {
					/* increase count */
					view_chain_count++;

					/* reallocate views here */
					view_chain_tmp = (char *)realloc(view_chain, model_chain_count * COMPONENT_NAME_LEN);
					if (!view_chain_tmp) {
						DEBUG_PRINT("Error: out of memory");
						/* TODO */
						sanji_routing_response(SESSION_CODE_SERVICE_UNAVAILABLE);
						if (model_chain) session_free_model_chain(model_chain, model_chain_count);
						if (!is_first_time && subscribed_component) free(subscribed_component);
						if (hook_component) free(hook_component);
						if (view_chain) free(view_chain);
						return 1;
					}
					view_chain = view_chain_tmp;
					memset(view_chain + (model_chain_count - 1) * COMPONENT_NAME_LEN, '\0', COMPONENT_NAME_LEN);
					memcpy(view_chain + (model_chain_count - 1) * COMPONENT_NAME_LEN, component->name, COMPONENT_NAME_LEN);
				}
			}
			if (!is_first_time) free(subscribed_component);
			is_first_time = 0;
		} while (hook_count);

		/* allocate result chain */
		result_chain = json_array();
		if (!result_chain) {
			DEBUG_PRINT("Error: out of memory");
			sanji_routing_response(SESSION_CODE_SERVICE_UNAVAILABLE);
			if (model_chain) session_free_model_chain(model_chain, model_chain_count);
			return 1;
		}

		/* create new session */
		session_new = session_create_node(id, method, topic, result_chain, model_chain, model_chain_count, view_chain, view_chain_count);
		if (!session_new) {
			DEBUG_PRINT("Error: out of memory");
			sanji_routing_response(SESSION_CODE_SERVICE_UNAVAILABLE);
			if (model_chain) session_free_model_chain(model_chain, model_chain_count);
			if (result_chain) json_decref(result_chain);
			return 1;
		}

		/* lock all related resource/models for write-like method */
		if (resource_is_write_like_method(session_new->method)) {
			ret = session_node_lock_by_step(session_new, sanji_resource, sanji_component, 0);
			if (ret) {
				DEBUG_PRINT("Error: session lock error.");
				/* TODO create a sanji error to response error mapping */
				//sanji_routing_response(SESSION_CODE_SERVICE_UNAVAILABLE);
				session_node_unlock_by_step(session_new, sanji_resource, sanji_component, 0);
				if (model_chain) session_free_model_chain(model_chain, model_chain_count);
				if (result_chain) json_decref(result_chain);
				if (session_new) session_free(session_new);
				return 1;
			}
		}

		/* linked to session list */
		session_add(sanji_session, session_new);

#ifdef DEBUG
		session_display(sanji_session);
		component_display(sanji_component);
		resource_display(sanji_resource);
#endif
		/* session start to step */
		session_step(session_new, sanji_session, sanji_resource, sanji_component, mosq, obj, root);

	} else {
		/* response procedure */
		DEBUG_PRINT("Response routing procedure");

		/* lookup session */
		session = session_lookup_node_by_id(sanji_session, id);
		if (!session) {
			DEBUG_PRINT("session is not inflight.");
			/* TODO */
			sanji_routing_response(SESSION_CODE_NOT_FOUND);
			return 1;
		}

		/* update session */
		session_step_update(session, sanji_session, sanji_resource, sanji_component, mosq, obj, root, code, data);
	}


	return 0;
}

int sanji_process_context(struct mosquitto *mosq, void *obj, char *context, unsigned int context_len)
{
	/* json context of sanji packet */
	json_t *root = NULL;			// root object
	int id = -1;					// '-1'	means 'no data'
	char method[RESOURCE_METHOD_LEN];
	char resource[RESOURCE_NAME_LEN];
	int code = -1;					// '-1' means 'no data'
	json_t *data = NULL;			// data object
	/* other var */
	json_error_t error;
	json_t *tmp = NULL;
	int method_num = -2;			// '-2' means 'no data'

	if (!mosq || !obj || !context || context_len <= 0 || context_len >= SANJI_MAX_CONTEXT_LEN) {
		DEBUG_PRINT("Error: empty context, should not be here.");
		return 1;
	}

	/* load json context */
	root = json_loads(context, 0, &error);
	if (!root) {
		DEBUG_PRINT("Error: wrong json format on line %d: %s", error.line, error.text);
		return 1;
	}

	/* get json context of packet */
	tmp = json_object_get(root, "id");
	if (tmp && json_is_number(tmp)) {
		id = json_number_value(tmp);
		DEBUG_PRINT("id: '%d'", id);
	}

	memset(method, '\0', RESOURCE_METHOD_LEN);
	tmp = json_object_get(root, "method");
	if (tmp && json_is_string(tmp)) {
		strncpy(method, json_string_value(tmp), RESOURCE_METHOD_LEN);
		method[RESOURCE_METHOD_LEN - 1] = '\0';
		method_num = resource_lookup_method(method);
		DEBUG_PRINT("method: '%s(%d)'", method, method_num);
	}

	memset(resource, '\0', RESOURCE_NAME_LEN);
	tmp = json_object_get(root, "resource");
	if (tmp && json_is_string(tmp)) {
		strncpy(resource, json_string_value(tmp), RESOURCE_NAME_LEN);
		resource[RESOURCE_NAME_LEN - 1] = '\0';
		DEBUG_PRINT("resource: '%s'", resource);
	}

	tmp = json_object_get(root, "code");
	if (tmp && json_is_number(tmp)) {
		code = json_number_value(tmp);
		DEBUG_PRINT("code: '%d'", code);
	}

	tmp = json_object_get(root, "data");
	if (tmp && json_is_object(tmp)) {
		data = tmp;
		DEBUG_PRINT("jason data object size %d\n", (int)json_object_size(data));
	}

	/* verify packet */
	if (id == -1) {
		DEBUG_PRINT("Error: wrong sanji packet, no 'id'.");
		json_decref(root);
		return 1;
	}
	if (code >= 0) {
		if ((method_num == -2) || (strlen(resource) <= 0)) {
			DEBUG_PRINT("Error: wrong sanji packet, no 'method' or 'resource'.");
			json_decref(root);
			return 1;
		}
	}

	/* start processing */
	if (!strncmp(SANJI_REGISTER_TOPIC, resource, SANJI_REGISTER_TOPIC_LEN)
			&& ((strlen(resource) == SANJI_REGISTER_TOPIC_LEN) || (resource[SANJI_REGISTER_TOPIC_LEN] == '/'))) {
		/* register prcedure */
		DEBUG_PRINT("register prcedure");
		sanji_register_procedure(mosq, obj, root, id, method_num, resource, code, data);
	} else {
		/* routing prcedure */
		DEBUG_PRINT("routing prcedure");
		sanji_routing_procedure(mosq, obj, root, id, method_num, resource, code, data);
	}

	DEBUG_PRINT("finish processing context");
	json_decref(root);

	return 0;
}

/*
 * ##########################
 * CALLBACK FUNCTIONS
 * ##########################
 */
void sanji_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	struct sanji_userdata *ud = NULL;

	assert(obj);
	ud = (struct sanji_userdata *)obj;

	if(message->retain && ud->no_retain) return;

	if(message->payloadlen){
		DEBUG_PRINT("get message len(%d), message(%s)", message->payloadlen, (char *)message->payload);
		sanji_process_context(mosq, obj, message->payload, message->payloadlen);
	}
}

void sanji_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	int i;
	struct sanji_userdata *ud = NULL;

	assert(obj);
	ud = (struct sanji_userdata *)obj;

	if(!result){
		for(i=0; i<ud->topic_count; i++){
			mosquitto_subscribe(mosq, NULL, ud->topics[i], ud->topic_qos);
		}
	}else{
		if(result && !ud->quiet){
			fprintf(stderr, "%s\n", mosquitto_connack_string(result));
		}
	}
}

void sanji_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	int i;
	struct sanji_userdata *ud = NULL;

	assert(obj);
	ud = (struct sanji_userdata *)obj;

	if(!ud->quiet) printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		if(!ud->quiet) printf(", %d", granted_qos[i]);
	}
	if(!ud->quiet) printf("\n");

	/* initiallize each list */
	sanji_resource = resource_init();
	sanji_component = component_init();
	sanji_session = session_init();
	if (!sanji_resource || !sanji_component || !sanji_session) {
		DEBUG_PRINT("Error: program initialization failed.");
		sanji_run = 0;
	}

}

void sanji_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	printf("SANJI: log_callback: %s\n", str);
}


/*
 * ##########################
 * MAIN FUNCTION
 * ##########################
 */
int main(int argc, char *argv[])
{
	/* sanji setup variable */
	struct mosquitto *mosq = NULL;
	struct sanji_userdata *ud = NULL;
	char sanji_register_topic[] = SANJI_REGISTER_TOPIC;
	bool clean_session = true;
	bool debug = false;
	char err[SANJI_ERR_BUFSIZE];
	/* client id */
	char id[SANJI_ID_LEN];
	char id_prefix[SANJI_ID_LEN];
	char hostname[SANJI_HOSTNAME_BUFSIZE];
	/* broker variable */
	char host[SANJI_IP_LEN] = SANJI_DEFAULT_IP;
	int port = SANJI_DEFAULT_PORT;
	int keepalive = SANJI_DEFAULT_KEEPALIVE;
	/* will information */
	char *will_topic = NULL;
	long will_payloadlen = 0;
	char *will_payload = NULL;
	int will_qos = 0;
	bool will_retain = false;
	/* temp variable */
	int i;
	int rc;

	/* initialized program and user data structure */
	ud = malloc(sizeof(struct sanji_userdata));
	memset(ud, 0, sizeof(struct sanji_userdata));
	memset(id, '\0', sizeof(id));
	memset(id_prefix, '\0', sizeof(id_prefix));

	/* default topic */
	ud->topic_count++;
	ud->topics = realloc(ud->topics, ud->topic_count*sizeof(char *));
	ud->topics[ud->topic_count-1] = sanji_register_topic;

	/* default publish configuration */
	ud->qos_sent = 2;

	/* get option */
	for(i=1; i<argc; i++){
		if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port")){
			if(i==argc-1){
				fprintf(stderr, "Error: -p argument given but no port specified.\n\n");
				sanji_print_usage();

				sanji_userdata_free(ud);
				return 1;
			}else{
				port = atoi(argv[i+1]);
				if(port<1 || port>65535){
					fprintf(stderr, "Error: Invalid port given: %d\n", port);
					sanji_print_usage();
					sanji_userdata_free(ud);
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "--disable-clean-session")){
			clean_session = false;
		}else if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")){
			debug = true;
		}else if(!strcmp(argv[i], "--help")){
			sanji_print_usage();
			sanji_userdata_free(ud);
			return 0;
		}else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--host")){
			if(i==argc-1){
				fprintf(stderr, "Error: -h argument given but no host specified.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}else{
				if (strlen(argv[i+1]) >= SANJI_IP_LEN) {
					fprintf(stderr, "Error: max length of ip is %d.\n\n", SANJI_IP_LEN);
					sanji_print_usage();
				} else {
					memset(host, '\0', sizeof(host));
					strcpy(host, argv[i+1]);
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--id")){
			if(strlen(id_prefix) != 0){
				fprintf(stderr, "Error: -i and -I argument cannot be used together.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}
			if(i==argc-1){
				fprintf(stderr, "Error: -i argument given but no id specified.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}else{
				if (strlen(argv[i+1]) >= MOSQ_MQTT_ID_MAX_LENGTH) {
					fprintf(stderr, "Error: max length of client id is %d.\n\n", MOSQ_MQTT_ID_MAX_LENGTH);
					sanji_print_usage();
				} else {
					strcpy(id, argv[i+1]);
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-I") || !strcmp(argv[i], "--id-prefix")){
			if(strlen(id) != 0){
				fprintf(stderr, "Error: -i and -I argument cannot be used together.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}
			if(i==argc-1){
				fprintf(stderr, "Error: -I argument given but no id prefix specified.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}else{
				if (strlen(argv[i+1]) >= MOSQ_MQTT_ID_MAX_LENGTH) {
					fprintf(stderr, "Error: max length of client id is %d.\n\n", MOSQ_MQTT_ID_MAX_LENGTH);
					sanji_print_usage();
				} else {
					strcpy(id_prefix, argv[i+1]);
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-k") || !strcmp(argv[i], "--keepalive")){
			if(i==argc-1){
				fprintf(stderr, "Error: -k argument given but no keepalive specified.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}else{
				keepalive = atoi(argv[i+1]);
				if(keepalive>65535){
					fprintf(stderr, "Error: Invalid keepalive given: %d\n", keepalive);
					sanji_print_usage();
					sanji_userdata_free(ud);
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-q") || !strcmp(argv[i], "--qos")){
			if(i==argc-1){
				fprintf(stderr, "Error: -q argument given but no QoS specified.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}else{
				ud->topic_qos = atoi(argv[i+1]);
				if(ud->topic_qos<0 || ud->topic_qos>2){
					fprintf(stderr, "Error: Invalid QoS given: %d\n", ud->topic_qos);
					sanji_print_usage();
					sanji_userdata_free(ud);
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "--quiet")){
			ud->quiet = true;
		}else if(!strcmp(argv[i], "-R")){
			ud->no_retain = true;
		}else if(!strcmp(argv[i], "-t") || !strcmp(argv[i], "--topic")){
			if(i==argc-1){
				fprintf(stderr, "Error: -t argument given but no topic specified.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}else{
				/* reset topic */
				if (ud->topics) {
					ud->topic_count = 0;
					free(ud->topics);
				}
				ud->topic_count++;
				ud->topics = realloc(ud->topics, ud->topic_count*sizeof(char *));
				ud->topics[ud->topic_count-1] = argv[i+1];
			}
			i++;
		}else if(!strcmp(argv[i], "-u") || !strcmp(argv[i], "--username")){
			if(i==argc-1){
				fprintf(stderr, "Error: -u argument given but no username specified.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}else{
				ud->username = argv[i+1];
			}
			i++;
		}else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")){
			ud->verbose = 1;
		}else if(!strcmp(argv[i], "-P") || !strcmp(argv[i], "--pw")){
			if(i==argc-1){
				fprintf(stderr, "Error: -P argument given but no password specified.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}else{
				ud->password = argv[i+1];
			}
			i++;
		}else if(!strcmp(argv[i], "--will-payload")){
			if(i==argc-1){
				fprintf(stderr, "Error: --will-payload argument given but no will payload specified.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}else{
				will_payload = argv[i+1];
				will_payloadlen = strlen(will_payload);
			}
			i++;
		}else if(!strcmp(argv[i], "--will-qos")){
			if(i==argc-1){
				fprintf(stderr, "Error: --will-qos argument given but no will QoS specified.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}else{
				will_qos = atoi(argv[i+1]);
				if(will_qos < 0 || will_qos > 2){
					fprintf(stderr, "Error: Invalid will QoS %d.\n\n", will_qos);
					sanji_userdata_free(ud);
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "--will-retain")){
			will_retain = true;
		}else if(!strcmp(argv[i], "--will-topic")){
			if(i==argc-1){
				fprintf(stderr, "Error: --will-topic argument given but no will topic specified.\n\n");
				sanji_print_usage();
				sanji_userdata_free(ud);
				return 1;
			}else{
				will_topic = argv[i+1];
			}
			i++;
		}else{
			fprintf(stderr, "Error: Unknown option '%s'.\n",argv[i]);
			sanji_print_usage();
			sanji_userdata_free(ud);
			return 1;
		}
	}

	/* verify necessary variable */
	if(clean_session == false && ((strlen(id_prefix) != 0) || (strlen(id) == 0))){
		if(!ud->quiet) fprintf(stderr, "Error: You must provide a client id if you are using the -c option.\n");
		sanji_userdata_free(ud);
		return 1;
	}
	if(ud->topic_count == 0){
		fprintf(stderr, "Error: You must specify a topic to subscribe to.\n");
		sanji_print_usage();
		sanji_userdata_free(ud);
		return 1;
	}
	if(will_payload && !will_topic){
		fprintf(stderr, "Error: Will payload given, but no will topic given.\n");
		sanji_print_usage();
		sanji_userdata_free(ud);
		return 1;
	}
	if(will_retain && !will_topic){
		fprintf(stderr, "Error: Will retain given, but no will topic given.\n");
		sanji_print_usage();
		sanji_userdata_free(ud);
		return 1;
	}
	if(ud->password && !ud->username){
		if(!ud->quiet) fprintf(stderr, "Warning: Not using password since username not set.\n");
	}

	/* setup signal handler */
	signal(SIGINT, sanji_signal_handler);
	signal(SIGTERM, sanji_signal_handler);

	/* init mosquitto library */
	mosquitto_lib_init();

	/* setup client id */
	if(strlen(id_prefix) != 0){
		snprintf(id, sizeof(id), "%s%d", id_prefix, getpid());
	}else if(strlen(id) == 0){
		memset(hostname, '\0', sizeof(hostname));
		gethostname(hostname, sizeof(hostname));
		snprintf(id, sizeof(id), "mosqsub/%d-%s", getpid(), hostname);
	}
	if(strlen(id) > MOSQ_MQTT_ID_MAX_LENGTH){
		/* Enforce maximum client id length of 23 characters */
		id[MOSQ_MQTT_ID_MAX_LENGTH] = '\0';
	}

	/* start mosquitto */
	mosq = mosquitto_new(id, clean_session, ud);
	if(!mosq){
		if(!ud->quiet) fprintf(stderr, "Error: %s\n", strerror(errno));
		mosquitto_lib_cleanup();
		sanji_userdata_free(ud);
		return 1;
	}

	/* setup mosquitto */
	if(debug){
		mosquitto_log_callback_set(mosq, sanji_log_callback);
	}
	if(will_topic && mosquitto_will_set(mosq, will_topic, will_payloadlen, will_payload, will_qos, will_retain)){
		if(!ud->quiet) fprintf(stderr, "Error: Problem setting will.\n");
		mosquitto_lib_cleanup();
		sanji_userdata_free(ud);
		return 1;
	}
	if(ud->username && mosquitto_username_pw_set(mosq, ud->username, ud->password)){
		if(!ud->quiet) fprintf(stderr, "Error: Problem setting username and password.\n");
		mosquitto_lib_cleanup();
		sanji_userdata_free(ud);
		return 1;
	}
	mosquitto_connect_callback_set(mosq, sanji_connect_callback);
	mosquitto_message_callback_set(mosq, sanji_message_callback);
	mosquitto_subscribe_callback_set(mosq, sanji_subscribe_callback);

	/* connect mosquitto */
	rc = mosquitto_connect(mosq, host, port, keepalive);
	if(rc){
		if(!ud->quiet){
			if(rc == MOSQ_ERR_ERRNO){
#ifndef WIN32
				strerror_r(errno, err, sizeof(err));
#else
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errno, 0, (LPTSTR)&err, sizeof(err), NULL);
#endif
				fprintf(stderr, "Error: %s\n", err);
			}else{
				fprintf(stderr, "Unable to connect (%d: %s).\n", rc, mosquitto_strerror(rc));
			}
		}
		mosquitto_lib_cleanup();
		sanji_userdata_free(ud);
		return rc;
	}

	/*
	 * loop mosquitto,
	 * it use select() to call back the callback-function which defined before.
	 */
	while (sanji_run) {
		rc = mosquitto_loop(mosq, SANJI_REFRESH_INTERVAL, 1);
		/* TODO: refresh ttl for each session */
		if (sanji_run && rc) {
			fprintf(stderr, "SANJI: reconnect to server\n");
			mosquitto_reconnect(mosq);
		}
	}

	/* free sanji */
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	sanji_userdata_free(ud);
	resource_free(sanji_resource);
	component_free(sanji_component);
	session_free(sanji_session);

	return 0;
}

