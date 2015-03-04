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
#include <time.h>
#ifndef WIN32
#  include <unistd.h>
#else
#  define _WIN32_WINNT _WIN32_WINNT_VISTA
#  include <windows.h>
#  include <process.h>
#  include <winsock2.h>
#  define snprintf sprintf_s
#endif
/* open source */
#include <mosquitto.h>
#include <jansson.h>
/* src */
#include "sanji_controller.h"
#include "ini.h"
#include "list.h"
#include "debug.h"
#include "resource.h"
#include "component.h"
#include "session.h"
/* lib */
#include "lock.h"
#include "pid.h"
#include "time_util.h"
#include "random_util.h"
#include "daemonize.h"


/*
 * ##########################
 * GLOBAL VARIABLES
 * ##########################
 */
struct mosquitto *mosq = NULL;
struct sanji_userdata *ud = NULL;
struct sanji_config *config = NULL;

static int sanji_run = 1;
static int sanji_dump = 0;

struct resource *sanji_resource = NULL;
struct component *sanji_component = NULL;
struct session *sanji_session = NULL;


/*
 * ##########################
 * SANJI SIGNAL HANDLER
 * ##########################
 */
void sanji_signal_quit(int s)
{
	DEBUG_PRINT("SIGNAL: get quit signal.");
	sanji_run = 0;
}

void sanji_signal_dump(int s)
{
	DEBUG_PRINT("SIGNAL: get dump signal.");
	sanji_dump = 1;
}

/*
 * ##########################
 * SANJI FUNCTIONS
 * ##########################
 */
void sanji_print_usage(void)
{
	printf("sanji_controller version %s.\n", SANJI_VERSION);
	printf("Usage: sanji_controller [-H host] [-p port] [-r retry] ...\n");
	printf("                        [-k keepalive] [-C] [-q sub_qos] [-Q pub_qos]\n");
	printf("                        [-u username] [-P password]\n");
	printf("                        [-i refresh_interval]\n");
	printf("                        [-c config_file] [-f] [-d]\n");
	printf("       sanji_controller [-h]\n\n");
	printf(" -H : mqtt host to connect to. Defaults to '%s'.\n", SANJI_DEFAULT_HOST);
	printf(" -p : network port to connect to. Defaults to %d.\n", SANJI_DEFAULT_PORT);
	printf(" -r : retry times to mqtt broker. Negative number for try-forever. Defaults to %d.\n", SANJI_DEFAULT_RETRY);
	printf(" -k : keep alive in seconds to mqtt broker. Defaults to %d.\n", SANJI_DEFAULT_KEEPALIVE);
	printf(" -C : disable 'clean session'.\n");
	printf(" -q : quality of service level to use for the subscription. Defaults to %d.\n", SANJI_DEFAULT_SUB_QOS);
	printf(" -Q : quality of service level to use for the publication. Defaults to %d.\n", SANJI_DEFAULT_PUB_QOS);
	printf(" -u : provide a username (requires MQTT 3.1 broker)\n");
	printf(" -P : provide a password (requires MQTT 3.1 broker)\n");
	printf(" -i : sanji session refresh interval in ms. Defualts to %d.\n", SANJI_DEFAULT_REFRESH_INTERVAL);
	printf(" -c : specify the sanji controller config file. Defaults to '%s'\n", SANJI_DEFAULT_CONFIG_FILE);
	printf(" -f : run in foreground.\n");
	printf(" -d : enable mosquitto debug messages.\n");
	printf(" -h : display this message.\n");
}

int sanji_get_cmdline_options(int argc, char *argv[], struct sanji_config *config)
{
	int c;

	while ((c = getopt(argc, argv, ":H:p:r:k:Cq:Q:u:P:i:c:fdh")) != -1) {
		switch (c) {
		case 'H':
			if (strlen(optarg) >= SANJI_IP_LEN) {
				fprintf(stderr, "ERROR: max length of ip is %d.\n", SANJI_IP_LEN - 1);
				return 1;
			} else {
				memset(config->host, '\0', SANJI_IP_LEN);
				strcpy(config->host, optarg);
			}
			break;
		case 'p':
			config->port = atoi(optarg);
			break;
		case 'r':
			config->retry = atoi(optarg);
			break;
		case 'k':
			config->keepalive = atoi(optarg);
			break;
		case 'C':
			config->clean_session = false;
			break;
		case 'q':
			config->sub_qos = atoi(optarg);
			break;
		case 'Q':
			config->pub_qos = atoi(optarg);
			break;
		case 'u':
			config->username = calloc(strlen(optarg) + 1, sizeof(char));
			strcpy(config->username, optarg);
			break;
		case 'P':
			config->password = calloc(strlen(optarg) + 1, sizeof(char));
			strcpy(config->password, optarg);
			break;
		case 'i':
			config->refresh_interval = atoi(optarg);
			break;
		case 'c':
			config->config_file = realloc(config->config_file, (strlen(optarg) + 1) * sizeof(char));
			memset(config->config_file, '\0', (strlen(optarg) + 1) * sizeof(char));
			strcpy(config->config_file, optarg);
			break;
		case 'f':
			config->foreground = true;
			break;
		case 'd':
			config->mosq_debug = true;
			break;
		case 'h':
			return 1;
		case ':':
		case '?':
		default:
			fprintf(stderr, "ERROR: incorrect options.\n");
			return 1;
		}
	}

	return 0;
}

int sanji_load_config_file(struct sanji_config *config)
{
	ini_t *ini = NULL;
	char value[SANJI_INI_VALUE_LEN];

	ini = ini_init(config->config_file);
	if (!ini) return 1;

#if (defined DEBUG) || (defined VERBOSE)
	fprintf(stderr, "SANJI: dump ini file:\n");
	ini_print(ini);
#endif

	if (!ini_findkey(ini, SANJI_INI_SECTION_GLOBAL, SANJI_INI_KEY_HOST, value, SANJI_INI_VALUE_LEN)) {
		if (strlen(value) >= SANJI_IP_LEN) {
			fprintf(stderr, "ERROR: max length of ip is %d.\n", SANJI_IP_LEN);
		} else {
			memset(config->host, '\0', SANJI_IP_LEN);
			strncpy(config->host, value, SANJI_IP_LEN - 1);
		}
	}
	if (!ini_findkey(ini, SANJI_INI_SECTION_GLOBAL, SANJI_INI_KEY_PORT, value, SANJI_INI_VALUE_LEN)) {
		config->port = atoi(value);
	}
	if (!ini_findkey(ini, SANJI_INI_SECTION_GLOBAL, SANJI_INI_KEY_RETRY, value, SANJI_INI_VALUE_LEN)) {
		config->retry = atoi(value);
	}
	if (!ini_findkey(ini, SANJI_INI_SECTION_GLOBAL, SANJI_INI_KEY_KEEPALIVE, value, SANJI_INI_VALUE_LEN)) {
		config->keepalive = atoi(value);
	}
	if (!ini_findkey(ini, SANJI_INI_SECTION_GLOBAL, SANJI_INI_KEY_CLEAN_SESSION, value, SANJI_INI_VALUE_LEN)) {
		config->clean_session = strcmp(value, "true") ? false : true;
	}
	if (!ini_findkey(ini, SANJI_INI_SECTION_GLOBAL, SANJI_INI_KEY_SUB_QOS, value, SANJI_INI_VALUE_LEN)) {
		config->sub_qos = atoi(value);
	}
	if (!ini_findkey(ini, SANJI_INI_SECTION_GLOBAL, SANJI_INI_KEY_PUB_QOS, value, SANJI_INI_VALUE_LEN)) {
		config->pub_qos = atoi(value);
	}
	if (!ini_findkey(ini, SANJI_INI_SECTION_GLOBAL, SANJI_INI_KEY_USERNAME, value, SANJI_INI_VALUE_LEN)) {
		if (strlen(value) > 0) {
			config->username = calloc(strlen(value) + 1, sizeof(char));
			strcpy(config->username, value);
		}
	}
	if (!ini_findkey(ini, SANJI_INI_SECTION_GLOBAL, SANJI_INI_KEY_PASSWORD, value, SANJI_INI_VALUE_LEN)) {
		if (strlen(value) > 0) {
			config->password = calloc(strlen(value) + 1, sizeof(char));
			strcpy(config->password, value);
		}
	}
	if (!ini_findkey(ini, SANJI_INI_SECTION_GLOBAL, SANJI_INI_KEY_REFRESH_INTERVAL, value, SANJI_INI_VALUE_LEN)) {
		config->refresh_interval = atoi(value);
	}
	if (!ini_findkey(ini, SANJI_INI_SECTION_GLOBAL, SANJI_INI_KEY_MOSQ_DEBUG, value, SANJI_INI_VALUE_LEN)) {
		config->mosq_debug = strcmp(value, "true") ? false : true;
	}

	ini_release(ini);

	return 0;
}

struct sanji_config *sanji_config_init()
{
	struct sanji_config *config;

	config = calloc(1, sizeof(struct sanji_config));
	if (!config) {
		fprintf(stderr, "ERROR: allocate memory failed.\n");
		return NULL;
	}

	/* set connect options */
	strcpy(config->host, SANJI_DEFAULT_HOST);
	config->port = SANJI_DEFAULT_PORT;
	config->retry = SANJI_DEFAULT_RETRY;
	/* set mosquitto options */
	config->keepalive = SANJI_DEFAULT_KEEPALIVE;
	config->clean_session = true;
	config->sub_qos = SANJI_DEFAULT_SUB_QOS;
	config->pub_qos = SANJI_DEFAULT_PUB_QOS;
	/* set controller options */
	config->refresh_interval = SANJI_DEFAULT_REFRESH_INTERVAL;
	/* set misc options */
	config->config_file = calloc(strlen(SANJI_DEFAULT_CONFIG_FILE) + 1, sizeof(char));
	strcpy(config->config_file, SANJI_DEFAULT_CONFIG_FILE);
	config->foreground = false;
	config->mosq_debug = false;

	return config;
}

void sanji_config_dump(struct sanji_config *config)
{
	fprintf(stderr, "SANJI: dump config:\n");
	/* connect */
	fprintf(stderr, "\thost = '%s'\n", config->host);
	fprintf(stderr, "\tport = %d\n", config->port);
	fprintf(stderr, "\tretry = %d\n", config->retry);
	/* mosquitto */
	fprintf(stderr, "\tkeepalive = %d\n", config->keepalive);
	fprintf(stderr, "\tclean_session = %s\n", config->clean_session ? "true" : "false");
	fprintf(stderr, "\tsub_qos = %d\n", config->sub_qos);
	fprintf(stderr, "\tpub_qos = %d\n", config->pub_qos);
	fprintf(stderr, "\tusername = %s\n", config->username ? config->username : "");
	fprintf(stderr, "\tpassword = %s\n", config->password ? config->password : "");
	/* controller */
	fprintf(stderr, "\trefresh_interval = %d\n", config->refresh_interval);
	/* misc */
	fprintf(stderr, "\tconfig_file = '%s'\n", config->config_file);
	fprintf(stderr, "\tforeground = %s\n", config->foreground ? "true" : "false");
	fprintf(stderr, "\tmosq_debug = %s\n", config->mosq_debug ? "true" : "false");
}

int sanji_validate_configs(struct sanji_config *config)
{
	if (config->port < 1 || config->port > 65535) {
		fprintf(stderr, "ERROR: Invalid port given: %d\n", config->port);
		return 1;
	}
	if (config->retry > 65535) {
		fprintf(stderr, "ERROR: Invalid retry given: %d\n", config->retry);
		return 1;
	}
	if (config->keepalive > 65535) {
		fprintf(stderr, "ERROR: Invalid keepalive given: %d\n", config->keepalive);
		return 1;
	}
	if (config->sub_qos < 0 || config->sub_qos > 2) {
		fprintf(stderr, "ERROR: Invalid QoS given: %d\n", config->sub_qos);
		return 1;
	}
	if (config->pub_qos < 0 || config->pub_qos > 2) {
		fprintf(stderr, "ERROR: Invalid QoS given: %d\n", config->pub_qos);
		return 1;
	}
	if (config->refresh_interval < 0 || config->refresh_interval > 65535) {
		fprintf(stderr, "ERROR: Invalid refresh_interval given: %d\n", config->refresh_interval);
		return 1;
	}
	if (config->password && !config->username) {
		fprintf(stderr, "WARNING: Not using password since username not set.\n");
	}

	return 0;
}

void sanji_config_free(struct sanji_config *config)
{
	if (config) {
		if (config->username) free(config->username);
		if (config->password) free(config->password);
		if (config->config_file) free(config->config_file);
		free(config);
	}
}

struct sanji_userdata *sanji_userdata_init()
{
	struct sanji_userdata *ud;

	ud = calloc(1, sizeof(struct sanji_userdata));
	if (!ud) {
		fprintf(stderr, "ERROR: allocate memory failed.\n");
		return NULL;
	}

	/* set client id */
	sprintf(ud->client_id, "sanji_controller");
	/* set topic to be listened */
	ud->topic_count++;
	ud->topics = realloc(ud->topics, ud->topic_count * sizeof(char *));
	ud->topics[ud->topic_count - 1] = SANJI_CONTROLLER_TOPIC;
	ud->topic_count++;
	ud->topics = realloc(ud->topics, ud->topic_count * sizeof(char *));
	ud->topics[ud->topic_count - 1] = SANJI_REGISTER_TOPIC;
	ud->topic_count++;
	ud->topics = realloc(ud->topics, ud->topic_count * sizeof(char *));
	ud->topics[ud->topic_count - 1] = SANJI_RESOURCE_DEPENDENCY_TOPIC;
	/* allocate topic mid */
	ud->topic_mids = (int *)malloc(ud->topic_count * sizeof(int));
	/* set qos of subscribe */
	ud->sub_qos = config->sub_qos;
	/* set qos of publish */
	ud->pub_qos = config->pub_qos;

	return ud;
}

void sanji_userdata_free(struct sanji_userdata *ud)
{
	if (ud) {
		if (ud->topics) free(ud->topics);
		if (ud->topic_mids) free(ud->topic_mids);
		free(ud);
	}
}

void sanji_dump_info()
{
	fprintf(stderr, "SANJI: dump component:\n");
	component_display(sanji_component);
	fprintf(stderr, "SANJI: dump resource:\n");
	resource_display(sanji_resource);
	fprintf(stderr, "SANJI: dump session:\n");
	session_display(sanji_session);
}

int _sanji_response(char *tunnel, char *context)
{
	int context_len;

	if (!tunnel || !context) return 1;

	/* publish a response by mosquitto */
	context_len = strlen(context);

#if (defined DEBUG) || (defined VERBOSE)
	DEBUG_PRINT("response to tunnel(%s)", tunnel);
	DEBUG_PRINT("%s", context);
#endif

	mosquitto_publish(mosq, &ud->mid_sent, tunnel, context_len, context, ud->pub_qos, ud->retain_sent);

	return 0;
}

int sanji_error_response(
		char *tunnel,
		unsigned int id,
		int method,
		char *topic,
		char *sign,
		int status_code,
		char *message,
		char *log)
{
	/* response context */
	json_t *response_root = NULL;
	json_t *response_sign = NULL;
	json_t *data = NULL;
	char *context = NULL;
	char *_method = NULL;
	int message_len = 0;
	int log_len = 0;

	if (!tunnel || (strlen(tunnel) <= 0)) return 1;

	/* create json response context */
	response_root = json_object();
	if (!response_root) {
		DEBUG_PRINT("ERROR: out of memory");
		return 1;
	}

	/* add 'id', 'code' */
	json_object_set_new(response_root, "id", json_integer(id));
	json_object_set_new(response_root, "code", json_integer(status_code));

	/* add 'method', 'resource' */
	_method = resource_reverse_method(method);
	if (_method) {
		json_object_set_new(response_root, "method", json_string(_method));
		free(_method);
	}
	if (topic) {
		json_object_set_new(response_root, "resource", json_string(topic));
	}

	/* add 'sign' */
	if (sign) {
		response_sign = json_array();
		json_array_append_new(response_sign, json_string(sign));
		json_object_set_new(response_root, "sign", response_sign);
	}

	/* add 'data' */
	if (message) message_len = strlen(message);
	if (log) log_len = strlen(log);
	if (message_len > 0 || log_len > 0) {
		data = json_object();
		if (!data) {
			DEBUG_PRINT("ERROR: out of memory");
			json_decref(response_root);
			return 1;
		}
		if (message_len > 0) json_object_set_new(data, "message", json_string(message));
		if (log_len > 0) json_object_set_new(data, "log", json_string(log));
		json_object_set_new(response_root, "data", data);
	}

	/* dump to string */
	context = json_dumps(response_root, JSON_INDENT(4));
	if (!context) {
		DEBUG_PRINT("ERROR: out of memory");
		json_decref(response_root);
		return 1;
	}
	json_decref(response_root);

	/* publish a response */
	_sanji_response(tunnel, context);
	free(context);

	return 0;
}

struct resource *sanji_match_resource(char *topic)
{
	struct resource *resource = NULL;
	char _resource[RESOURCE_NAME_LEN];
	int match_alg = SANJI_MATCH_ALG_EXACT;
	char *p = NULL;

	/*
	 * Match rules:
	 *   1. truncate '?'
	 *   2. match resource to topic.
	 */
	memset(_resource, '\0', RESOURCE_NAME_LEN);
	strncpy(_resource, topic, RESOURCE_NAME_LEN);

	/* truncate '?' */
	p = strrchr(_resource, '?');
	if (p) *p = '\0';

	/* match resource to topic */
	resource = resource_lookup_node_by_name(sanji_resource, _resource);
	if (match_alg == SANJI_MATCH_ALG_LONGEST) {
		while (!resource) {
			/* truncate the last fragment */
			p = strrchr(_resource, '/');
			if (p) {
				*p = '\0';
				resource = resource_lookup_node_by_name(sanji_resource, _resource);
			} else {
				break;
			}
		}
	}

	return resource;
}

struct resource **sanji_match_resource_list(char *topic, unsigned int *resource_list_count)
{
	/* inspect var */
	int is_duplicate = 0;
	/* to allocate request data var */
	struct resource **resource_list = NULL;
	struct resource *resource = NULL;
	unsigned int resource_count = 0;
	/* temp var */
	struct resource **resource_list_tmp = NULL;
	int i;

	resource = sanji_match_resource(topic);
	if (resource) resource_count++;

	/* Lookup resource list for wildcard '#' and '+' */
	resource_list = resource_lookup_wildcard_nodes_by_name(sanji_resource, topic, resource_list_count); // MUST FREE return address

	/* combine resource and wildcard resources */
	if (!resource && !resource_list) {
		return NULL;
	}

	is_duplicate = 0;
	if (resource_count) {
		for (i = 0; i < *resource_list_count; i++) {
			if (!strncmp(resource->name, resource_list[i]->name, RESOURCE_NAME_LEN)) {
				is_duplicate = 1;
				break;
			}
		}
	}
	if (!is_duplicate && resource_count) {
		*resource_list_count += resource_count;
		resource_list_tmp = realloc(resource_list, *resource_list_count * sizeof(struct resource *));
		if (!resource_list_tmp) {
			DEBUG_PRINT("Error: out of memory");
			if (resource_list) free(resource_list);
			return NULL;
		}
		resource_list = resource_list_tmp;
		resource_list[*resource_list_count - resource_count] = resource;
	}
#if (defined DEBUG) || (defined VERBOSE)
	DEBUG_PRINT("resource list(%d)", *resource_list_count);
	for (i = 0; i < *resource_list_count; i++) {
		DEBUG_PRINT("resource_list(%s)", resource_list[i]->name);
	}
#endif

	return resource_list;
}

char *sanji_get_subscribed_component(struct resource **resource_list, unsigned int resource_list_count, unsigned int *subscribed_count)
{
	/* to allocate request data var */
	char *subscribed_component = NULL;
	/* temp var */
	struct resource *resource = NULL;
	unsigned int _subscribed_count = 0;
	char *subscribed_component_tmp = NULL;
	int i;

	for (i = 0; i < resource_list_count; i++) {
		/* for each resource in resource list */
		resource = resource_list[i];

		/* allocate subscribed component */
		_subscribed_count = resource_get_subscribed_count(resource);
		if (_subscribed_count) {
			*subscribed_count += _subscribed_count;
			subscribed_component_tmp = (char *)realloc(subscribed_component, *subscribed_count * COMPONENT_NAME_LEN);
			if (!subscribed_component_tmp) {
				DEBUG_PRINT("ERROR: out of memory");
				if (subscribed_component) free(subscribed_component);
				return NULL;
			}
			subscribed_component = subscribed_component_tmp;
			memset(subscribed_component + (*subscribed_count - _subscribed_count) * COMPONENT_NAME_LEN, '\0', _subscribed_count * COMPONENT_NAME_LEN);
			memcpy(subscribed_component + (*subscribed_count - _subscribed_count) * COMPONENT_NAME_LEN, resource_get_subscribed_component(resource), _subscribed_count * COMPONENT_NAME_LEN);
		}
	}

	return subscribed_component;
}

/*
 * SANJI REGISTER PROCEDURE
 */
int register_error_response(
		unsigned int id,
		int method,
		char *topic,
		int status_code,
		char *message,
		char *log)
{
	sanji_error_response(SANJI_CONTROLLER_TOPIC, id, method, topic, SANJI_CONTROLLER_NAME, status_code, message, log);

	return 0;
}

int register_response(
		unsigned int id,
		int method,
		char *topic,
		int status_code,
		char *tunnel)
{
	/* response context */
	json_t *response_root = NULL;
	json_t *response_sign = NULL;
	json_t *data = NULL;
	char *_method = NULL;
	char *context = NULL;

	/* create json response context */
	response_root = json_object();
	if (!response_root) {
		DEBUG_PRINT("ERROR: out of memory");
		return 1;
	}

	/* add 'id' and 'code' */
	json_object_set_new(response_root, "id", json_integer(id));
	json_object_set_new(response_root, "code", json_integer(status_code));

	/* add 'method', 'resource' */
	_method = resource_reverse_method(method);
	if (_method) {
		json_object_set_new(response_root, "method", json_string(_method));
		free(_method);
	}
	if (topic) {
		json_object_set_new(response_root, "resource", json_string(topic));
	}

	/* add 'sign' */
	response_sign = json_array();
	json_array_append_new(response_sign, json_string(SANJI_CONTROLLER_NAME));
	json_object_set_new(response_root, "sign", response_sign);

	/* add 'data' */
	data = json_object();
	if (!data) {
		DEBUG_PRINT("ERROR: out of memory");
		json_decref(response_root);
		return 1;
	}
	json_object_set_new(data, "tunnel", json_string(tunnel));
	json_object_set_new(response_root, "data", data);

	/* dump to string */
	context = json_dumps(response_root, JSON_INDENT(4));
	if (!context) {
		DEBUG_PRINT("ERROR: out of memory");
		json_decref(response_root);
		return 1;
	}
	json_decref(response_root);

	/* publish response */
	_sanji_response(SANJI_CONTROLLER_TOPIC, context);

	free(context);

	return 0;
}

int register_create(char *name, char *description, char *role, char *hook, unsigned int hook_count, int ttl, char *resources, unsigned int resources_count, char *tunnel, char *message, char *log)
{
	int is_registered = 0;
	int is_locked = 0;
	int i;

	memset(message, '\0', SANJI_MESSAGE_LEN);
	memset(log, '\0', SANJI_MESSAGE_LEN);

	if (!name || !description || !role || !resources) {
		strncpy(message, "internal server error", SANJI_MESSAGE_LEN);
		return SESSION_CODE_INTERNAL_SERVER_ERROR;
	}

	/* check whether if one of resources is locked */
	for (i = 0; i < resources_count; i++) {
		is_locked += resource_is_locked(sanji_resource, resources + i * COMPONENT_NAME_LEN);
	}
	if (is_locked) {
		DEBUG_PRINT("ERROR: resoures is locked.");
		strncpy(message, "the resource is locked", SANJI_MESSAGE_LEN);
		return SESSION_CODE_LOCKED;
	}

	/*
	 * create component object
	 */
	/* check whether already been registered */
	is_registered = component_is_registered(sanji_component, name);
	if (is_registered == -1) {
		strncpy(message, "internal server error", SANJI_MESSAGE_LEN);
		return SESSION_CODE_INTERNAL_SERVER_ERROR;
	} else if (is_registered) {
		DEBUG_PRINT("ERROR: component has already been registered.");
		strncpy(message, "component had beed already registered", SANJI_MESSAGE_LEN);
		return SESSION_CODE_FORBIDDEN;
	}

	/* create a unique random tunnel */
	do {
		memset(tunnel, '\0', COMPONENT_TUNNEL_LEN);
		sprintf(tunnel, "%d", generate_random(RAND_MODE_RANDOM));
	} while (!component_is_unique_tunnel(sanji_component, tunnel));

	if (component_add_node(sanji_component, name, description, tunnel, role, hook, hook_count, ttl, 0)) {
		DEBUG_PRINT("ERROR: out of memory");
		strncpy(message, "some service unavailable", SANJI_MESSAGE_LEN);
		return SESSION_CODE_SERVICE_UNAVAILABLE;
	}

#if (defined DEBUG) || (defined VERBOSE)
	DEBUG_PRINT("dump components:");
	component_display(sanji_component);
#endif

	/*
	 * create resource object
	 */
	for (i = 0; i < resources_count; i++) {
		resource_append_component_by_name(sanji_resource, resources + i * RESOURCE_NAME_LEN, name);
	}

#if (defined DEBUG) || (defined VERBOSE)
	DEBUG_PRINT("dump resources:");
	resource_display(sanji_resource);
#endif

	return SESSION_CODE_OK;
}

int register_delete(char *name, char *message, char *log)
{
	int is_locked = 0;
	int is_registered = 0;

	memset(message, '\0', SANJI_MESSAGE_LEN);
	memset(log, '\0', SANJI_MESSAGE_LEN);

	if (!name) {
		strncpy(message, "internal server error", SANJI_MESSAGE_LEN);
		return SESSION_CODE_INTERNAL_SERVER_ERROR;
	}

	/* check whether component had been registered */
	is_registered = component_is_registered(sanji_component, name);
	if (is_registered == -1) {
		strncpy(message, "internal server error", SANJI_MESSAGE_LEN);
		return SESSION_CODE_INTERNAL_SERVER_ERROR;
	} else if (!is_registered) {
		DEBUG_PRINT("'%s' has NOT been registered.", name);
		return SESSION_CODE_OK;
	}

	/* check whether if one of resources/components is locked */
	is_locked += component_is_locked(sanji_component, name);
	is_locked += resource_is_any_locked_by_component(sanji_resource, name);
	if (is_locked) {
		DEBUG_PRINT("ERROR: resoures is locked.");
		strncpy(message, "resoures is locked", SANJI_MESSAGE_LEN);
		return SESSION_CODE_LOCKED;
	}

	/* delete component */
	if (component_delete_first_name(sanji_component, name)) {
		DEBUG_PRINT("ERROR: out of memory");
		strncpy(message, "internal server error", SANJI_MESSAGE_LEN);
		return SESSION_CODE_INTERNAL_SERVER_ERROR;
	}

#if (defined DEBUG) || (defined VERBOSE)
	DEBUG_PRINT("dump components:");
	component_display(sanji_component);
#endif

	/*
	 * delete resource object
	 */
	resource_remove_all_component_by_name(sanji_resource, name);

#if (defined DEBUG) || (defined VERBOSE)
	DEBUG_PRINT("dump resources:");
	resource_display(sanji_resource);
#endif

	return SESSION_CODE_OK;
}

int register_procedure(
		json_t *root,
		unsigned int id,
		int method,
		char *topic,
		json_t *sign,
		int code,
		json_t *data)
{
	/* request data */
	char name[COMPONENT_NAME_LEN];
	char description[COMPONENT_DESCRIPTION_LEN];
	char role[COMPONENT_ROLE_LEN];
	json_t *_hook = NULL;			// hook array
	char *hook = NULL;
	unsigned int hook_count = 0;
	int ttl;
	json_t *_resources = NULL;
	char *resources = NULL;
	unsigned int resources_count = 0;
	/* response data */
	int status_code;
	char tunnel[COMPONENT_TUNNEL_LEN];
	char message[SANJI_MESSAGE_LEN];
	char log[SANJI_MESSAGE_LEN];
	/* others */
	json_t *tmp = NULL;
	int i;

	DEBUG_PRINT("[REGISTER]");

	/*
	 * Validate context content
	 * 	1. 'resource' is not '/controller/registration'
	 * 	2. 'CREATE' method with no 'data'.
	 */
	if ((strlen(topic) < SANJI_REGISTER_RESOURCE_LEN)
			|| (strncmp(topic, SANJI_REGISTER_RESOURCE, SANJI_REGISTER_RESOURCE_LEN))) {
		DEBUG_PRINT("ERROR: resource didn't match.");
		register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "'resource' didn't match");
	}
	if (method == RESOURCE_METHOD_CREATE && !data) {
		DEBUG_PRINT("ERROR: 'CREATE' method with no 'data'.");
		register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "Need 'data'");
		return 1;
	}

	/* acquire 'name' from topic or data */
	memset(name, '\0', COMPONENT_NAME_LEN);
	if (method == RESOURCE_METHOD_CREATE || method == RESOURCE_METHOD_DELETE) {
		if (strlen(topic) > SANJI_REGISTER_RESOURCE_LEN) {
			if (topic[SANJI_REGISTER_RESOURCE_LEN] == '/') {
				strncpy(name, &topic[SANJI_REGISTER_RESOURCE_LEN + 1], COMPONENT_NAME_LEN);
				name[COMPONENT_NAME_LEN - 1] = '\0';
#if defined DEBUG
				DEBUG_PRINT("name: '%s'", name);
#endif
			} else {
				DEBUG_PRINT("ERROR: resource didn't match.");
				register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "'resource' didn't match");
				return 1;
			}
		} else {
			/* use 'name' in attribute */
			tmp = json_object_get(data, "name");
			if (tmp && json_is_string(tmp) && (strlen(json_string_value(tmp)) > 0)) {
				strncpy(name, json_string_value(tmp), COMPONENT_NAME_LEN);
				name[COMPONENT_NAME_LEN - 1] = '\0';
#if defined DEBUG
				DEBUG_PRINT("name: '%s'", name);
#endif
			} else {
				DEBUG_PRINT("ERROR: wrong sanji packet, no 'name'.");
				register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "no 'name'");
				return 1;
			}
		}
	}

	/* CRUD methods */
	switch (method) {

	case RESOURCE_METHOD_CREATE:
		DEBUG_PRINT("[REGISTER] create method");

		/* acquire 'description' */
		memset(description, '\0', COMPONENT_DESCRIPTION_LEN);
		tmp = json_object_get(data, "description");
		if (tmp && json_is_string(tmp) && (strlen(json_string_value(tmp)) > 0)) {
			strncpy(description, json_string_value(tmp), COMPONENT_DESCRIPTION_LEN);
			description[COMPONENT_DESCRIPTION_LEN - 1] = '\0';
#if defined DEBUG
			DEBUG_PRINT("description: '%s'", description);
#endif
		} else {
			DEBUG_PRINT("ERROR: wrong sanji packet, no 'description'.");
			register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "need 'description'");
			return 1;
		}

		/* acquire 'role' */
		memset(role, '\0', COMPONENT_ROLE_LEN);
		tmp = json_object_get(data, "role");
		if (tmp && json_is_string(tmp) && (strlen(json_string_value(tmp)) > 0)) {
			strncpy(role, json_string_value(tmp), COMPONENT_ROLE_LEN);
			role[COMPONENT_ROLE_LEN - 1] = '\0';
#if defined DEBUG
			DEBUG_PRINT("role: '%s'", role);
#endif
		} else {
			DEBUG_PRINT("ERROR: wrong sanji packet, no 'role'.");
			register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "need 'role'");
			return 1;
		}

		/* acquire 'hook' */
		/* Note, After acquire 'hook', we must free it before return. */
		_hook = json_object_get(data, "hook");
		if (_hook) {
			if (json_is_array(_hook)) {
				hook_count = json_array_size(_hook);
				if (hook_count) {
					hook = (char *)malloc(hook_count * sizeof(char) * COMPONENT_NAME_LEN);
					if (!hook) {
						DEBUG_PRINT("ERROR: out of memory");
						register_error_response(id, method, topic, SESSION_CODE_SERVICE_UNAVAILABLE, "service unavailable", NULL);
						return 1;
					}
					for (i = 0; i < hook_count; i++) {
						tmp = json_array_get(_hook, i);
						if (tmp && json_is_string(tmp) && (strlen(json_string_value(tmp)) > 0)) {
							strncpy(hook + (i * COMPONENT_NAME_LEN), json_string_value(tmp), COMPONENT_NAME_LEN);
							hook[(i + 1) * COMPONENT_NAME_LEN - 1] = '\0';
						} else {
							DEBUG_PRINT("ERROR: wrong sanji packet, empty 'hook'.");
							register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "empty 'hook'");
							if (hook) free(hook);
							return 1;
						}
					}
				}
			} else if (json_is_string(_hook)) {
				hook_count = 1;
				hook = (char *)malloc(hook_count * sizeof(char) * COMPONENT_NAME_LEN);
				if (strlen(json_string_value(_hook)) > 0) {
					strncpy(hook, json_string_value(_hook), COMPONENT_NAME_LEN);
					hook[COMPONENT_NAME_LEN - 1] = '\0';
				} else {
					DEBUG_PRINT("ERROR: wrong sanji packet, empty 'hook'.");
					register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "empty 'hook'");
					if (hook) free(hook);
					return 1;
				}
			}
		}

		/* acquire 'ttl' */
		tmp = json_object_get(data, "ttl");
		if (tmp && json_is_number(tmp)) {
			ttl = json_number_value(tmp);
#if defined DEBUG
			DEBUG_PRINT("ttl: '%d'", ttl);
#endif
		} else {
			DEBUG_PRINT("ERROR: wrong sanji packet, no 'ttl'.");
			register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "need 'ttl'");
			if (hook) free(hook);
			return 1;
		}

		/* acquire 'resources' */
		/* Note, after acquire 'resources', we must free it before return. */
		_resources = json_object_get(data, "resources");
		if (_resources) {
			if (json_is_array(_resources)) {
				resources_count = json_array_size(_resources);
				if (!resources_count) {
					DEBUG_PRINT("ERROR: wrong sanji packet, empty 'resources'.");
					register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "empty 'resources'");
					if (hook) free(hook);
					return 1;
				}
				resources = (char *)malloc(resources_count * sizeof(char) * RESOURCE_NAME_LEN);
				if (!resources) {
					DEBUG_PRINT("ERROR: out of memory");
					if (hook) free(hook);
					register_error_response(id, method, topic, SESSION_CODE_SERVICE_UNAVAILABLE, "service unavailable", NULL);
					return 1;
				}
				for (i = 0; i < resources_count; i++) {
					tmp = json_array_get(_resources, i);
					if (tmp && json_is_string(tmp) && (strlen(json_string_value(tmp)) > 0)) {
						strncpy(resources + (i * RESOURCE_NAME_LEN), json_string_value(tmp), RESOURCE_NAME_LEN);
						resources[(i + 1) * RESOURCE_NAME_LEN - 1] = '\0';
					} else {
						DEBUG_PRINT("ERROR: wrong sanji packet, empty 'resources'.");
						register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "empty 'resources'");
						if (hook) free(hook);
						if (resources) free(resources);
						return 1;
					}
				}
			} else if (json_is_string(_resources)) {
				resources_count = 1;
				resources = (char *)malloc(resources_count * sizeof(char) * RESOURCE_NAME_LEN);
				if (strlen(json_string_value(_resources)) > 0) {
					strncpy(resources, json_string_value(_resources), RESOURCE_NAME_LEN);
					resources[RESOURCE_NAME_LEN - 1] = '\0';
				} else {
					DEBUG_PRINT("ERROR: wrong sanji packet, empty 'resources'.");
					register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "empty 'resources'");
					if (hook) free(hook);
					if (resources) free(resources);
					return 1;
				}
			}
		} else {
			DEBUG_PRINT("ERROR: wrong sanji packet, no 'resources'.");
			register_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong register context", "Need 'resources'");
			if (hook) free(hook);
			return 1;
		}

		/* create registeration */
		status_code = register_create(name, description, role, hook, hook_count, ttl, resources, resources_count, tunnel, message, log);
		if (status_code == SESSION_CODE_OK) {
			register_response(id, method, topic, status_code, tunnel);
		} else {
			register_error_response(id, method, topic, status_code, message, log);
		}

		break;

	case RESOURCE_METHOD_READ:
		DEBUG_PRINT("[REGISTER] read method");

		/* TODO: support read method */

		/* read registeration */
		DEBUG_PRINT("read method is not implemented.");
		register_error_response(id, method, topic, SESSION_CODE_METHOD_NOT_ALLOWED, "method not allowed", NULL);

		break;

	case RESOURCE_METHOD_UPDATE:
		DEBUG_PRINT("[REGISTER] update method");

		/* update registeration */
		DEBUG_PRINT("update method is not implemented.");
		register_error_response(id, method, topic, SESSION_CODE_METHOD_NOT_ALLOWED, "method not allowed", NULL);

		break;

	case RESOURCE_METHOD_DELETE:
		DEBUG_PRINT("[REGISTER] delete method");

		/* delete registeration */
		status_code = register_delete(name, message, log);
		register_error_response(id, method, topic, status_code, message, log);

		break;

	default:
		DEBUG_PRINT("[REGISTER] unknown method");
		register_error_response(id, method, topic, SESSION_CODE_METHOD_NOT_ALLOWED, "method not allowed", NULL);
		break;
	}

	/* clear */
	if (hook) free(hook);
	if (resources) free(resources);

	return 0;
}

/*
 * SANJI LOOKUP RESOURCE DEPENDENCY PROCEDURE
 */
int dependency_error_response(
		unsigned int id,
		int method,
		char *topic,
		int status_code,
		char *message,
		char *log)
{
	sanji_error_response(SANJI_CONTROLLER_TOPIC, id, method, topic, SANJI_CONTROLLER_NAME, status_code, message, log);

	return 0;
}

int dependency_response(
		unsigned int id,
		int method,
		char *topic,
		int status_code,
		char *resources,
		unsigned int resources_count)
{
	/* response context */
	json_t *response_root = NULL;
	json_t *response_sign = NULL;
	json_t *data = NULL;
	json_t *_resources = NULL;
	char *_method = NULL;
	char *context = NULL;
	int i;

	/* create json response context */
	response_root = json_object();
	if (!response_root) {
		DEBUG_PRINT("ERROR: out of memory");
		return 1;
	}

	/* add 'id' and 'code' */
	json_object_set_new(response_root, "id", json_integer(id));
	json_object_set_new(response_root, "code", json_integer(status_code));

	/* add 'method', 'resource' */
	_method = resource_reverse_method(method);
	if (_method) {
		json_object_set_new(response_root, "method", json_string(_method));
		free(_method);
	}
	if (topic) {
		json_object_set_new(response_root, "resource", json_string(topic));
	}

	/* add 'sign' */
	response_sign = json_array();
	json_array_append_new(response_sign, json_string(SANJI_CONTROLLER_NAME));
	json_object_set_new(response_root, "sign", response_sign);

	/* create 'data' */
	data = json_object();
	if (!data) {
		DEBUG_PRINT("ERROR: out of memory");
		json_decref(response_root);
		return 1;
	}

	/* add 'resources' */
	_resources = json_array();
	if (!_resources) {
		DEBUG_PRINT("ERROR: out of memory");
		json_decref(response_root);
		json_decref(data);
		return 1;
	}
	for (i = 0; i < resources_count; i++) {
		json_array_append_new(_resources, json_string(&resources[i * RESOURCE_NAME_LEN]));
	}
	json_object_set_new(data, "resources", _resources);
	json_object_set_new(response_root, "data", data);

	/* dump to string */
	context = json_dumps(response_root, JSON_INDENT(4));
	if (!context) {
		DEBUG_PRINT("ERROR: out of memory");
		json_decref(response_root);
		return 1;
	}
	json_decref(response_root);

	/* publish response */
	_sanji_response(SANJI_CONTROLLER_TOPIC, context);

	free(context);

	return 0;
}

int dependency_read(char *resource, char **resources, unsigned int *resources_count, char *message, char *log)
{
	/* resource */
	struct resource **resource_list = NULL;
	unsigned int resource_list_count = 0;
	char *_resources = NULL;
	unsigned int last_resources_count = 0;
	/* component */
	struct component *component = NULL;
	char *subscribed_component = NULL;
	unsigned int subscribed_count = 0;
	/* hook */
	char *hook_component = NULL;
	char *_hook_component = NULL;
	unsigned int hook_count = 0;
	unsigned int last_hook_count = 0;
	/* inspect var */
	int is_model = 0;
	int is_duplicate = 0;
	/* temp var */
	char *resources_tmp = NULL;
	char *hook_tmp = NULL;
	int i, j, k;

	*resources_count = 0;
	memset(message, '\0', SANJI_MESSAGE_LEN);
	memset(log, '\0', SANJI_MESSAGE_LEN);

	if (!resource) {
		strncpy(message, "internal server error", SANJI_MESSAGE_LEN);
		return SESSION_CODE_INTERNAL_SERVER_ERROR;
	}

	/* match resource list */
	resource_list = sanji_match_resource_list(resource, &resource_list_count); // MUST FREE return address
	if (!resource_list) {
		DEBUG_PRINT("ERROR: Resource(%s) had not been registered", resource);
		strncpy(message, "resource had not been registered", SANJI_MESSAGE_LEN);
		return SESSION_CODE_NOT_FOUND;
	}

	/* get subscribed component */
	subscribed_component = sanji_get_subscribed_component(resource_list, resource_list_count, &subscribed_count);
	if (!subscribed_component) {
		DEBUG_PRINT("Resource not implemented.");
		strncpy(message, "resource is not implemented", SANJI_MESSAGE_LEN);
		if (resource_list) free(resource_list);
		return SESSION_CODE_NOT_IMPLEMENTED;
	}
	if (resource_list) free(resource_list);

	/*
	 * Start to find model chain and dependency chain
	 */
	do {
#if (defined DEBUG) || (defined VERBOSE)
		DEBUG_PRINT("subscribed_count(%d)", subscribed_count);
		for (i = 0; i < subscribed_count; i++) {
			DEBUG_PRINT("subscribed_component(%s)", subscribed_component + i * COMPONENT_NAME_LEN);
		}
#endif
		/* reallocate models/views */
		for (i = 0; i < subscribed_count; i++) {
			/* for each component */
			component = component_lookup_node_by_name(sanji_component, subscribed_component + i * COMPONENT_NAME_LEN);
			if (!component) {
				DEBUG_PRINT("Component didn't be registered.");
				continue;
			}

			/* for each model */
			is_model = component_node_is_given_role(component, "model");
			if (is_model) {
				/* reallocate resources and resources count */
				last_resources_count = *resources_count;
				_resources = resource_get_names_by_component(sanji_resource, component->name, resources_count); // MUST FREE return address
				if (_resources) {
					/* search current 'resources' for each entry of '_resources' to avoid duplicate entry */
					for (j = 0; j < *resources_count; j++) {
						is_duplicate = 0;
						for (k = 0; k < last_resources_count; k++) {
							if (!strncmp(_resources + j * RESOURCE_NAME_LEN, *resources + k * RESOURCE_NAME_LEN, RESOURCE_NAME_LEN)) {
								is_duplicate = 1;
								break;
							}
						}
						if (!is_duplicate) {
							last_resources_count++;
							resources_tmp = (char *)realloc(*resources, last_resources_count * RESOURCE_NAME_LEN);
							if (!resources_tmp) {
								DEBUG_PRINT("ERROR: out of memory");
								strncpy(message, "service unavailable", SANJI_MESSAGE_LEN);
								if (subscribed_component) free(subscribed_component);
								if (hook_component) free(hook_component);
								if (*resources) free(*resources);
								if (_resources) free(_resources);
								return SESSION_CODE_SERVICE_UNAVAILABLE;
							}
							*resources = resources_tmp;
							memset(*resources + (last_resources_count - 1) * RESOURCE_NAME_LEN, '\0', RESOURCE_NAME_LEN);
							memcpy(*resources + (last_resources_count - 1) * RESOURCE_NAME_LEN, _resources + j * RESOURCE_NAME_LEN, RESOURCE_NAME_LEN);
						}
					}
					free(_resources);
				}
				*resources_count = last_resources_count;

				/* reallocate hook models and hook count */
				last_hook_count = hook_count;
				_hook_component = component_get_names_by_hook(sanji_component, component->name, &hook_count); // MUST FREE return address
				if (_hook_component) {
					hook_tmp = (char *)realloc(hook_component, (last_hook_count + hook_count) * COMPONENT_NAME_LEN);
					if (!hook_tmp) {
						DEBUG_PRINT("ERROR: out of memory");
						strncpy(message, "service unavailable", SANJI_MESSAGE_LEN);
						if (subscribed_component) free(subscribed_component);
						if (hook_component) free(hook_component);
						if (*resources) free(*resources);
						if (_hook_component) free(_hook_component);
						return SESSION_CODE_SERVICE_UNAVAILABLE;
					}
					hook_component = hook_tmp;
					memset(hook_component + last_hook_count * COMPONENT_NAME_LEN, '\0', hook_count * COMPONENT_NAME_LEN);
					memcpy(hook_component + last_hook_count * COMPONENT_NAME_LEN, _hook_component, hook_count * COMPONENT_NAME_LEN);
					free(_hook_component);
				}
				hook_count += last_hook_count;
			}
		}

		/* clean temp variable */
		if (subscribed_component) {
			free(subscribed_component);
			subscribed_component = NULL;
		}
		subscribed_component = hook_component;
		subscribed_count = hook_count;
		hook_component = NULL;
		hook_count = 0;

	} while (subscribed_count);

	/* clear */
	if (subscribed_component) free(subscribed_component);
	if (hook_component) free(hook_component);

	return SESSION_CODE_OK;
}

int lookup_resource_dependency_procedure(
		json_t *root,
		unsigned int id,
		int method,
		char *topic,
		json_t *sign,
		int code,
		json_t *data)
{
	/* request data */
	char resource[RESOURCE_NAME_LEN];
	char *resources = NULL;
	unsigned int resources_count = 0;
	/* response data */
	int status_code;
	char message[SANJI_MESSAGE_LEN];
	char log[SANJI_MESSAGE_LEN];
	/* others */
	int topic_len;

	DEBUG_PRINT("[RESOURCE DEPENDENCY]");

	topic_len = strlen(topic);

	/*
	 * Validate context content
	 * 	1. 'resource' is not '/controller/resource/dependency'
	 */
	if ((topic_len < SANJI_RESOURCE_DEPENDENCY_RESOURCE_LEN)
			|| (strncmp(topic, SANJI_RESOURCE_DEPENDENCY_RESOURCE, SANJI_RESOURCE_DEPENDENCY_RESOURCE_LEN))) {
		DEBUG_PRINT("ERROR: resource didn't match.");
		dependency_error_response(id, method, topic, SESSION_CODE_BAD_REQUEST, "wrong context", "'resource' didn't match");
	}

	/* CRUD methods */
	switch (method) {

	case RESOURCE_METHOD_CREATE:
		DEBUG_PRINT("[RESOURCE DEPENDENCY] create method");

		DEBUG_PRINT("create method is not implemented.");
		dependency_error_response(id, method, topic, SESSION_CODE_METHOD_NOT_ALLOWED, "method not allowed", NULL);

		break;

	case RESOURCE_METHOD_READ:
		DEBUG_PRINT("[RESOURCE DEPENDENCY] read method");

		memset(resource, '\0', RESOURCE_NAME_LEN);
		if (topic_len == SANJI_RESOURCE_DEPENDENCY_RESOURCE_LEN) {
			/* read capability */
			DEBUG_PRINT("ERROR: didn't support read capability");
			dependency_error_response(id, method, topic, SESSION_CODE_METHOD_NOT_ALLOWED, "method not allowed", "not support read capability");
		}

		/* acquire 'resource' from query string in topic */
		if (topic[SANJI_RESOURCE_DEPENDENCY_RESOURCE_LEN] == '?') {
			if (!strncmp(topic + SANJI_RESOURCE_DEPENDENCY_RESOURCE_LEN, "?resource=", 10)) {
				strncpy(resource, &topic[SANJI_RESOURCE_DEPENDENCY_RESOURCE_LEN + 10], RESOURCE_NAME_LEN);
				resource[RESOURCE_NAME_LEN - 1] = '\0';
				DEBUG_PRINT("resource: '%s'", resource);
			}
		}

		if (strlen(resource) <= 0) {
			DEBUG_PRINT("ERROR: cannot get 'resource'");
			dependency_error_response(id, method, topic, SESSION_CODE_FORBIDDEN, "query forbidden", "only allow with query string '?resource=[identifier]'");
			return 1;
		}

		status_code = dependency_read(resource, &resources, &resources_count, message, log); // MUST FREE 'resources'

		if (status_code == SESSION_CODE_OK) {
			dependency_response(id, method, topic, status_code, resources, resources_count);
		} else {
			dependency_error_response(id, method, topic, status_code, message, log);
		}

		break;

	case RESOURCE_METHOD_UPDATE:
		DEBUG_PRINT("[RESOURCE DEPENDENCY] update method");

		DEBUG_PRINT("update method is not implemented.");
		dependency_error_response(id, method, topic, SESSION_CODE_METHOD_NOT_ALLOWED, "method not allowed", NULL);

		break;

	case RESOURCE_METHOD_DELETE:
		DEBUG_PRINT("[RESOURCE DEPENDENCY] delete method");

		DEBUG_PRINT("delete method is not implemented.");
		dependency_error_response(id, method, topic, SESSION_CODE_METHOD_NOT_ALLOWED, "method not allowed", NULL);

		break;

	default:
		DEBUG_PRINT("[RESOURCE DEPENDENCY] unknown method");
		dependency_error_response(id, method, topic, SESSION_CODE_METHOD_NOT_ALLOWED, "method not allowed", NULL);
		break;
	}

	/* clear */
	if (resources) free(resources);

	DEBUG_PRINT("[RESOURCE DEPENDENCY] finish lookup resource dependency procedure");

	return 0;
}

/*
 * SANJI ROUTING PROCEDURE
 */
int routing_error_response(
		struct resource *resource,
		unsigned int id,
		int method,
		char *topic,
		int status_code,
		char *message,
		char *log)
{
	char *subscribed_component = NULL;
	unsigned int subscribed_count;
	char *component = NULL;
	char *tunnel = NULL;
	int i;

	if (!resource) return 1;

	subscribed_component = resource_get_subscribed_component(resource);
	subscribed_count = resource_get_subscribed_count(resource);
	if (!subscribed_component || !subscribed_count) {
		DEBUG_PRINT("Resource has NOT been registered.");
		return 1;
	}

	/* find 'tunnel' of each view component */
	for (i = 0; i < subscribed_count; i++) {
		component = subscribed_component + i * COMPONENT_NAME_LEN;
		if (component_is_given_role(sanji_component, component, "view")) {
			tunnel = component_get_tunnel_by_name(sanji_component, component);
			sanji_error_response(tunnel, id, method, topic, SANJI_CONTROLLER_NAME, status_code, message, log);
		}
	}

	return 0;
}

int routing_error_response_all(
		char *tunnel,
		struct resource *resource[],
		unsigned int resource_count,
		unsigned int id,
		int method,
		char *topic,
		int status_code,
		char *message,
		char *log)
{
	int i;
	int ret;

	/* if we get 'tunnel', ONLY response to this 'tunnel' */
	if (tunnel && (strlen(tunnel) > 0)) {
		sanji_error_response(tunnel, id, method, topic, SANJI_CONTROLLER_NAME, status_code, message, log);
		return 0;
	}

	for (i = 0; i < resource_count; i++) {
		ret = routing_error_response(resource[i], id, method, topic, status_code, message, log);
		if (ret) return ret;
	}

	return 0;
}

int routing_event(json_t *root, char *topic)
{
	struct resource **resource_list = NULL;
	unsigned int resource_list_count = 0;
	char *subscribed_component = NULL;
	unsigned int subscribed_count = 0;
	char *component = NULL;
	char *tunnel = NULL;
	char *context = NULL;
	int i;

	DEBUG_PRINT("[EVENT]");

	/* dump to string */
	context = json_dumps(root, JSON_INDENT(4));
	if (!context) {
		DEBUG_PRINT("ERROR: out of memory");
		return 1;
	}

	/* get resource list */
	resource_list = sanji_match_resource_list(topic, &resource_list_count); // MUST FREE return address
	if (!resource_list) {
		DEBUG_PRINT("Resource had not been registered.");
		if (context) free(context);
		return 1;
	}

	/* get subscribed component */
	subscribed_component = sanji_get_subscribed_component(resource_list, resource_list_count, &subscribed_count);
	if (!subscribed_component) {
		DEBUG_PRINT("Resource not implemented.");
		if (context) free(context);
		if (resource_list) free(resource_list);
		return 1;
	}

	/* broadcast event to all 'view' */
	for (i = 0; i < subscribed_count; i++) {
		component = subscribed_component + i * COMPONENT_NAME_LEN;
		if (component_is_given_role(sanji_component, component, "view")) {
			tunnel = component_get_tunnel_by_name(sanji_component, component);
			_sanji_response(tunnel, context);
		}
	}

	if (context) free(context);
	if (resource_list) free(resource_list);
	if (subscribed_component) free(subscribed_component);

	return 0;
}

int routing_req(
		json_t *root,
		unsigned int id,
		int method,
		char *topic,
		char *tunnel,
		int code,
		json_t *data)
{
	/* request data */
	struct session *session_new = NULL;
	struct model_chain *model_chain = NULL;
	unsigned int model_chain_count = 0;
	char *view_chain = NULL;
	int view_count = 0;
	json_t *result_chain = NULL;
	char *dependency_chain = NULL;
	char *_dependency_chain = NULL;
	unsigned int dependency_count = 0;
	unsigned int last_dependency_count = 0;
	/* inspect var */
	int is_inflight = 0;
	int is_locked = 0;
	int is_model = 0;
	int is_view = 0;
	int is_duplicate = 0;
	/* to allocate request data var */
	struct component *component = NULL;
	struct resource **resource_list = NULL;
	unsigned int resource_list_count = 0;
	struct resource *resource = NULL;
	char *subscribed_component = NULL;
	unsigned int subscribed_count = 0;
	char *hook_component = NULL;
	char *_hook_component = NULL;
	unsigned int hook_count = 0;
	unsigned int last_hook_count = 0;
	/* temp var */
	struct model_chain *model_chain_tmp = NULL;
	char *models_tmp = NULL;
	char *dependency_chain_tmp = NULL;
	int *ttls_tmp = NULL;
	char *hook_tmp = NULL;
	char *view_chain_tmp = NULL;
	int i, j, k;

	DEBUG_PRINT("[REQUEST]");

	/*
	 * Match resource list first,
	 * since we get the capability to response after acquiring the resource.
	 */
	resource_list = sanji_match_resource_list(topic, &resource_list_count); // MUST FREE return address
	if (!resource_list) {
		DEBUG_PRINT("Resource had not been registered.");
		routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_NOT_FOUND, NULL, NULL);
		return 1;
	}

	/* is session inflight */
	is_inflight = session_is_inflight(sanji_session, id);
	if (is_inflight < 0) {
		DEBUG_PRINT("ERROR: sanji session crash.");
		routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_INTERNAL_SERVER_ERROR, "internal server error", NULL);
		if (resource_list) free(resource_list);
		return 1;
	} else if (is_inflight) {
		DEBUG_PRINT("Session is inflight.");
		routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_FORBIDDEN, "'id' was used, please use another 'id'", NULL);
		if (resource_list) free(resource_list);
		return 1;
	}

	/* is any resource locked */
	for (i = 0; i < resource_list_count; i++) {
		resource = resource_list[i];

		is_locked += resource_node_is_locked(resource);
		if (is_locked) {
			DEBUG_PRINT("Resource is locked.");
			routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_LOCKED, "resource is locked", NULL);
			if (resource_list) free(resource_list);
			return 1;
		}
	}

	/* get subscribed component */
	subscribed_component = sanji_get_subscribed_component(resource_list, resource_list_count, &subscribed_count);
	if (!subscribed_component) {
		DEBUG_PRINT("Resource not implemented.");
		routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_NOT_IMPLEMENTED, "resource is not implemented", NULL);
		if (resource_list) free(resource_list);
		return 1;
	}

	/*
	 * Start to find model chain and view chain.
	 */
	do {
#if (defined DEBUG) || (defined VERBOSE)
		DEBUG_PRINT("subscribed_count(%d)", subscribed_count);
		for (i = 0; i < subscribed_count; i++) {
			DEBUG_PRINT("subscribed_component(%s)", subscribed_component + i * COMPONENT_NAME_LEN);
		}
#endif
		/* increase model chain counts */
		model_chain_count++;

		/* reallocate model chains */
		model_chain_tmp = (struct model_chain *)realloc(model_chain, model_chain_count * sizeof(struct model_chain));
		if (!model_chain_tmp) {
			DEBUG_PRINT("ERROR: out of memory");
			routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_SERVICE_UNAVAILABLE, "service unavailable", NULL);
			if (model_chain) session_free_model_chain(model_chain, model_chain_count - 1);
			if (subscribed_component) free(subscribed_component);
			if (hook_component) free(hook_component);
			if (dependency_chain) free(dependency_chain);
			if (view_chain) free(view_chain);
			if (resource_list) free(resource_list);
			return 1;
		}
		model_chain = model_chain_tmp;

		/* reuse model_chain_tmp as model_chain index */
		model_chain_tmp = &model_chain[model_chain_count - 1];
		memset(model_chain_tmp, 0, sizeof(struct model_chain));

		/* reallocate models/views */
		for (i = 0; i < subscribed_count; i++) {
			/* for each component */
			component = component_lookup_node_by_name(sanji_component, subscribed_component + i * COMPONENT_NAME_LEN);
			if (!component) {
				DEBUG_PRINT("Component didn't be registered.");
				continue;
			}

			/* for each model */
			is_model = component_node_is_given_role(component, "model");
			if (is_model) {
				/* check model locked */
				is_locked = component_node_is_locked(component);
				if (is_locked) {
					DEBUG_PRINT("ERROR: model is busy.");
					routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_LOCKED, "resource is locked", NULL);
					if (model_chain) session_free_model_chain(model_chain, model_chain_count);
					if (subscribed_component) free(subscribed_component);
					if (hook_component) free(hook_component);
					if (dependency_chain) free(dependency_chain);
					if (view_chain) free(view_chain);
					if (resource_list) free(resource_list);
					return 1;
				}

				/* increase model counts */
				model_chain_tmp->count++;

				/* reallocate models */
				models_tmp = (char *)realloc(model_chain_tmp->models, model_chain_tmp->count * COMPONENT_NAME_LEN);
				if (!models_tmp) {
					DEBUG_PRINT("ERROR: out of memory");
					routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_SERVICE_UNAVAILABLE, "service unavailable", NULL);
					if (model_chain) session_free_model_chain(model_chain, model_chain_count);
					if (subscribed_component) free(subscribed_component);
					if (hook_component) free(hook_component);
					if (dependency_chain) free(dependency_chain);
					if (view_chain) free(view_chain);
					if (resource_list) free(resource_list);
					return 1;
				}
				model_chain_tmp->models = models_tmp;
				memset(model_chain_tmp->models + (model_chain_tmp->count - 1) * COMPONENT_NAME_LEN, '\0', COMPONENT_NAME_LEN);
				memcpy(model_chain_tmp->models + (model_chain_tmp->count - 1) * COMPONENT_NAME_LEN, component->name, COMPONENT_NAME_LEN);

				/* reallocate ttls */
				ttls_tmp = (int *)realloc(model_chain_tmp->ttls, model_chain_tmp->count * sizeof(int));
				if (!ttls_tmp) {
					DEBUG_PRINT("ERROR: out of memory");
					routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_SERVICE_UNAVAILABLE, "service unavailable", NULL);
					if (model_chain) session_free_model_chain(model_chain, model_chain_count);
					if (subscribed_component) free(subscribed_component);
					if (hook_component) free(hook_component);
					if (dependency_chain) free(dependency_chain);
					if (view_chain) free(view_chain);
					if (resource_list) free(resource_list);
					return 1;
				}
				model_chain_tmp->ttls = ttls_tmp;
				model_chain_tmp->ttls[model_chain_tmp->count - 1] = component->ttl;

				if (resource_is_write_like_method(method)) {
					/* reallocate dependency resources and dependency count for write-like method */
					last_dependency_count = dependency_count;
					_dependency_chain = resource_get_names_by_component(sanji_resource, component->name, &dependency_count); // MUST FREE return address
					if (_dependency_chain) {
						/* search current 'dependency_chain' for each entry of '_dependency_chain' to avoid duplicate entry */
						for (j = 0; j < dependency_count; j++) {
							is_duplicate = 0;
							for (k = 0; k < last_dependency_count; k++) {
								if (!strncmp(_dependency_chain + j * RESOURCE_NAME_LEN, dependency_chain + k * RESOURCE_NAME_LEN, RESOURCE_NAME_LEN)) {
									is_duplicate = 1;
									break;
								}
							}
							if (!is_duplicate) {
								last_dependency_count++;
								dependency_chain_tmp = (char *)realloc(dependency_chain, last_dependency_count * RESOURCE_NAME_LEN);
								if (!dependency_chain_tmp) {
									DEBUG_PRINT("ERROR: out of memory");
									routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_SERVICE_UNAVAILABLE, "service unavailable", NULL);
									if (model_chain) session_free_model_chain(model_chain, model_chain_count);
									if (subscribed_component) free(subscribed_component);
									if (hook_component) free(hook_component);
									if (dependency_chain) free(dependency_chain);
									if (view_chain) free(view_chain);
									if (_dependency_chain) free(_dependency_chain);
									if (resource_list) free(resource_list);
									return 1;
								}
								dependency_chain = dependency_chain_tmp;
								memset(dependency_chain + (last_dependency_count - 1) * RESOURCE_NAME_LEN, '\0', RESOURCE_NAME_LEN);
								memcpy(dependency_chain + (last_dependency_count - 1) * RESOURCE_NAME_LEN, _dependency_chain + j * RESOURCE_NAME_LEN, RESOURCE_NAME_LEN);
							}
						}
						free(_dependency_chain);
					}
					dependency_count = last_dependency_count;

					/* reallocate hook models and hook count for write-like method */
					last_hook_count = hook_count;
					_hook_component = component_get_names_by_hook(sanji_component, component->name, &hook_count); // MUST FREE return address
					if (_hook_component) {
						hook_tmp = (char *)realloc(hook_component, (last_hook_count + hook_count) * COMPONENT_NAME_LEN);
						if (!hook_tmp) {
							DEBUG_PRINT("ERROR: out of memory");
							routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_SERVICE_UNAVAILABLE, "service unavailable", NULL);
							if (model_chain) session_free_model_chain(model_chain, model_chain_count);
							if (subscribed_component) free(subscribed_component);
							if (hook_component) free(hook_component);
							if (dependency_chain) free(dependency_chain);
							if (view_chain) free(view_chain);
							if (_hook_component) free(_hook_component);
							if (resource_list) free(resource_list);
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

			/* for each view */
			is_view = component_node_is_given_role(component, "view");
			if (is_view) {
				/* increase view counts */
				view_count++;

				/* reallocate views */
				view_chain_tmp = (char *)realloc(view_chain, view_count * COMPONENT_NAME_LEN);
				if (!view_chain_tmp) {
					DEBUG_PRINT("ERROR: out of memory");
					routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_SERVICE_UNAVAILABLE, "service unavailable", NULL);
					if (model_chain) session_free_model_chain(model_chain, model_chain_count);
					if (subscribed_component) free(subscribed_component);
					if (hook_component) free(hook_component);
					if (dependency_chain) free(dependency_chain);
					if (view_chain) free(view_chain);
					if (resource_list) free(resource_list);
					return 1;
				}
				view_chain = view_chain_tmp;
				memset(view_chain + (view_count - 1) * COMPONENT_NAME_LEN, '\0', COMPONENT_NAME_LEN);
				memcpy(view_chain + (view_count - 1) * COMPONENT_NAME_LEN, component->name, COMPONENT_NAME_LEN);
			}
		}

		/* clean temp variable */
		if (subscribed_component) {
			free(subscribed_component);
			subscribed_component = NULL;
		}
		subscribed_component = hook_component;
		subscribed_count = hook_count;
		hook_component = NULL;
		hook_count = 0;

	} while (subscribed_count);

	/* 
	 * If resouces doesn't be subscribed by any model.
	 * It could happen since resource is subscribed by views only.
	 */
	if (!model_chain->count) {
		DEBUG_PRINT("ERROR: Resource has no subscribed model.");
		routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_FORBIDDEN, "request forbidden", "resource has no subscribed model");
		if (model_chain) session_free_model_chain(model_chain, model_chain_count);
		if (dependency_chain) free(dependency_chain);
		if (view_chain) free(view_chain);
		if (resource_list) free(resource_list);
		return 1;
	}

	/* allocate result chain */
	result_chain = json_array();
	if (!result_chain) {
		DEBUG_PRINT("ERROR: out of memory");
		routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_SERVICE_UNAVAILABLE, "service unavailable", NULL);
		if (model_chain) session_free_model_chain(model_chain, model_chain_count);
		if (dependency_chain) free(dependency_chain);
		if (view_chain) free(view_chain);
		if (resource_list) free(resource_list);
		return 1;
	}

	/* create new session */
	session_new = session_create_node(id, method, topic, tunnel, dependency_chain, dependency_count, result_chain, model_chain, model_chain_count, view_chain, view_count);
	if (!session_new) {
		DEBUG_PRINT("ERROR: out of memory");
		routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_SERVICE_UNAVAILABLE, "service unavailable", NULL);
		if (model_chain) session_free_model_chain(model_chain, model_chain_count);
		if (dependency_chain) free(dependency_chain);
		if (view_chain) free(view_chain);
		if (result_chain) json_decref(result_chain);
		if (resource_list) free(resource_list);
		return 1;
	}

	/* lock all related resource/models for write-like method */
	if (resource_is_write_like_method(session_new->method)) {
		if (session_node_lock_by_step(session_new, sanji_resource, sanji_component, 0)) {
			DEBUG_PRINT("ERROR: session lock error.");
			routing_error_response_all(tunnel, resource_list, resource_list_count, id, method, topic, SESSION_CODE_SERVICE_UNAVAILABLE, "service unavailable", "failed to lock resource");
			session_node_unlock_by_step(session_new, sanji_resource, sanji_component, 0);
			if (model_chain) session_free_model_chain(model_chain, model_chain_count);
			if (view_chain) free(view_chain);
			if (dependency_chain) free(dependency_chain);
			if (result_chain) json_decref(result_chain);
			if (session_new) session_free(session_new);
			if (resource_list) free(resource_list);
			return 1;
		}
	}

	/* linked to session list */
	session_add(sanji_session, session_new);

	/* free resource list */
	if (resource_list) free(resource_list);

#if (defined DEBUG) || (defined VERBOSE)
	DEBUG_PRINT("dump sessions:");
	session_display(sanji_session);
	DEBUG_PRINT("dump componpents:");
	component_display(sanji_component);
	DEBUG_PRINT("dump resources:");
	resource_display(sanji_resource);
#endif

	/* start to step session */
	session_step(session_new, sanji_session, sanji_resource, sanji_component, mosq, ud, root);

	return 0;
}

int routing_res(json_t *root, unsigned int id, int code, json_t *data)
{
	struct session *session = NULL;

	DEBUG_PRINT("[RESPONSE]");

	/* lookup session */
	session = session_lookup_node_by_id(sanji_session, id);
	if (!session) {
		DEBUG_PRINT("session is not inflight.");
		return 1;
	}

	/* update session */
	session_step_update(session, sanji_session, sanji_resource, sanji_component, mosq, ud, root, code, data);

	return 0;
}

int routing_procedure(
		json_t *root,
		unsigned int id,
		int method,
		char *topic,
		char *tunnel,
		json_t *sign,
		int code,
		json_t *data)
{
	DEBUG_PRINT("[ROUTING]");

	/* validate arguments */
	if (!root || !topic) {
		DEBUG_PRINT("ERROR: missing necessary arguments.");
		return 1;
	}

	/*
	 * Sanji communication type:
	 * 1. event
	 * 2. request/response
	 */
	if (id == SANJI_HEADER_NO_ID) {
		/* event procedure */
		routing_event(root, topic);
	} else if (code == SANJI_HEADER_NO_CODE) {
		/* request procedure */
		routing_req(root, id, method, topic, tunnel, code, data);
	} else {
		/* response procedure */
		routing_res(root, id, code, data);
	}

	return 0;
}

int sanji_parse_context(
		char *context,
		json_t **root,
		unsigned int *id,
		int *method,
		char *resource,
		char *tunnel,
		json_t **sign,
		int *code,
		json_t **data)
{
	json_error_t error;
	json_t *tmp = NULL;
	char _method[RESOURCE_METHOD_LEN];

	if (!context) return SANJI_INTERNAL_ERROR;

	/* load json context */
	*root = json_loads(context, 0, &error);
	if (!(*root)) {
		DEBUG_PRINT("ERROR: wrong json context format on line %d: %s", error.line, error.text);
		return SANJI_DATA_ERROR;
	}

	/* get id from json context */
	tmp = json_object_get(*root, "id");
	if (tmp && json_is_number(tmp)) {
		*id = json_number_value(tmp);
#if defined DEBUG
		DEBUG_PRINT("id: '%u'", *id);
#endif
	}

	/* get method from json context */
	memset(_method, '\0', RESOURCE_METHOD_LEN);
	tmp = json_object_get(*root, "method");
	if (tmp && json_is_string(tmp)) {
		strncpy(_method, json_string_value(tmp), RESOURCE_METHOD_LEN);
		_method[RESOURCE_METHOD_LEN - 1] = '\0';
		*method = resource_lookup_method(_method);
#if defined DEBUG
		DEBUG_PRINT("method: '%s(%d)'", _method, *method);
#endif
	}

	/* get resource from json context */
	memset(resource, '\0', RESOURCE_NAME_LEN);
	tmp = json_object_get(*root, "resource");
	if (tmp && json_is_string(tmp)) {
		strncpy(resource, json_string_value(tmp), RESOURCE_NAME_LEN);
		resource[RESOURCE_NAME_LEN - 1] = '\0';
#if defined DEBUG
		DEBUG_PRINT("resource: '%s'", resource);
#endif
	}

	/* get tunnel from json context */
	memset(tunnel, '\0', COMPONENT_TUNNEL_LEN);
	tmp = json_object_get(*root, "tunnel");
	if (tmp && json_is_string(tmp)) {
		strncpy(tunnel, json_string_value(tmp), COMPONENT_TUNNEL_LEN);
		tunnel[COMPONENT_TUNNEL_LEN - 1] = '\0';
#if defined DEBUG
		DEBUG_PRINT("tunnel: '%s'", tunnel);
#endif
	}

	/* get sign from json context */
	tmp = json_object_get(*root, "sign");
	if (tmp && json_is_array(tmp)) {
		*sign = tmp;
#if defined DEBUG
		DEBUG_PRINT("sign: array size %d", (int)json_array_size(*sign));
#endif
	}

	/* get code from json context */
	tmp = json_object_get(*root, "code");
	if (tmp && json_is_number(tmp)) {
		*code = json_number_value(tmp);
#if defined DEBUG
		DEBUG_PRINT("code: '%d'", *code);
#endif
	}

	/* get data from json context */
	tmp = json_object_get(*root, "data");
	if (tmp && json_is_object(tmp)) {
		*data = tmp;
#if defined DEBUG
		DEBUG_PRINT("data: object size %d", (int)json_object_size(*data));
#endif
	}

	return SANJI_SUCCESS;
}

int sanji_dispatch_context(char *topic, char *context, unsigned int context_len)
{
	/* sanji context content */
	json_t *root = NULL;
	unsigned int id = SANJI_HEADER_NO_ID;
	int method = SANJI_HEADER_NO_METHOD;
	char resource[RESOURCE_NAME_LEN] = { 0 };
	char tunnel[COMPONENT_TUNNEL_LEN] = { 0 };
	json_t *sign = NULL;
	int code = SANJI_HEADER_NO_CODE;
	json_t *data = NULL;

	DEBUG_PRINT("[DISPATCH]");

	/* validate arguments */
	if (!context || context_len <= 0 || context_len >= SANJI_MAX_CONTEXT_LEN) {
		DEBUG_PRINT("ERROR: context is empty or too long.");
		return SANJI_INTERNAL_ERROR;
	}

	/*
	 * Parse arguments to sanji context content
	 *
	 * This function will allocate 'json_t *root',
	 * must free it in the end
	 */
	if (sanji_parse_context(context, &root, &id, &method, resource, tunnel, &sign, &code, &data)) {
		DEBUG_PRINT("ERROR: failed to parse sanji context content.");
		if (root) json_decref(root);
		return SANJI_DATA_ERROR;
	}

	/*
	 * validate context content:
	 * 	1. event context with no method or no resource
	 * 	2. request context with no method or no resource
	 */
	if (id == SANJI_HEADER_NO_ID) {
		if ((method == SANJI_HEADER_NO_METHOD) || (strlen(resource) <= 0)) {
			DEBUG_PRINT("ERROR: wrong sanji context content, request context with no method or no resource.");
			if (root) json_decref(root);
			return SANJI_DATA_ERROR;
		}
	}
	if (code == SANJI_HEADER_NO_CODE) {
		if ((method == SANJI_HEADER_NO_METHOD) || (strlen(resource) <= 0)) {
			DEBUG_PRINT("ERROR: wrong sanji context content, request context with no method or no resource.");
			if (root) json_decref(root);
			return SANJI_DATA_ERROR;
		}
	}

	/*
	 * start processing context
	 * 	1. routing procedure (/controller)
	 *  2. register procedure (/controller/registration)
	 *	3. lookup resource dependency procedure (/controller/resource/dependency)
	 */
	if (!strcmp(topic, SANJI_CONTROLLER_TOPIC)) {
		routing_procedure(root, id, method, resource, tunnel, sign, code, data);
	} else if (!strcmp(topic, SANJI_REGISTER_TOPIC)) {
		register_procedure(root, id, method, resource, sign, code, data);
	} else if (!strcmp(topic, SANJI_RESOURCE_DEPENDENCY_TOPIC)) {
		lookup_resource_dependency_procedure(root, id, method, resource, sign, code, data);
	}

	json_decref(root);

	return SANJI_SUCCESS;
}

void sanji_refresh_session()
{
	static time_t last_time = 0;
	static time_t now_time = 0;
	static unsigned int diff_time = 0;
	static char timestamp[TIMESTAMP_LEN];

	get_timestamp(TIMESTAMP_MODE_MONOTONIC, timestamp, TIMESTAMP_LEN);
	now_time = atol(timestamp);
	//DEBUG_PRINT("now time(%ld)", now_time);
	if (last_time) {
		diff_time = (unsigned int)(now_time - last_time);
		if (diff_time) {
			session_decref_ttl(sanji_session, sanji_resource, sanji_component, mosq, ud, diff_time);
		}
	}
	last_time = now_time;
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

	if (message->retain) return;

	if (message->payloadlen) {
		DEBUG_PRINT("========================================================");
		DEBUG_PRINT("[MESSAGE]");
		DEBUG_PRINT("topic(%s) mid(%d) qos(%d) message(%d):\n%s",
				message->topic,
				message->mid,
				message->qos,
				message->payloadlen,
				(char *)message->payload);

		/* START POINT */
		sanji_dispatch_context(message->topic, message->payload, message->payloadlen);
	}
}

void sanji_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	int i;
	struct sanji_userdata *ud = NULL;
	char *topic = NULL;

	assert(obj);
	ud = (struct sanji_userdata *)obj;

	/* acquire topic from mid */
	for (i = 0; i < ud->topic_count; i++) {
		if (ud->topic_mids[i] == mid) {
			topic = ud->topics[i];
			break;
		}
	}

	fprintf(stderr, "SANJI: Subscribed mid(%d), topic('%s'), qos(%d", mid, topic, granted_qos[0]);
	for (i = 1; i < qos_count; i++){
		fprintf(stderr, ", %d", granted_qos[i]);
	}
	fprintf(stderr, ")\n");
}

void sanji_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	int i;
	struct sanji_userdata *ud = NULL;

	assert(obj);
	ud = (struct sanji_userdata *)obj;

	if (!result) {
		for (i = 0; i < ud->topic_count; i++) {
			mosquitto_subscribe(mosq, &ud->topic_mids[i], ud->topics[i], ud->sub_qos);
		}
	} else {
		fprintf(stderr, "%s\n", mosquitto_connack_string(result));
	}

	/*
	 * init sanji controller object
	 *
	 * Reconnect will trigger this callback routine.
	 * If controller already allocate object, don't allocate again.
	 */
	fprintf(stderr, "SANJI: initializing sanji controller.\n");
	if (!sanji_resource) sanji_resource = resource_init();
	if (!sanji_component) sanji_component = component_init();
	if (!sanji_session) sanji_session = session_init();

	if (!sanji_resource || !sanji_component || !sanji_session) {
		fprintf(stderr, "ERROR: failed to initialize sanji controller.\n");
		sanji_run = 0;
	}

	/*
	 * Register sanji controller build-in model
	 *
	 * Reconnect will trigger this callback routine.
	 * If build-in model already be registerd, don't register again.
	 */
	/* 'registration' model */
	if (!component_is_registered(sanji_component, SANJI_REGISTER_NAME)) {
		component_add_node(sanji_component, SANJI_REGISTER_NAME, "This is registration model.", SANJI_REGISTER_TOPIC, "model", NULL, 0, 10, 0);
		resource_append_component_by_name(sanji_resource, SANJI_REGISTER_RESOURCE, SANJI_REGISTER_NAME);
		resource_append_component_by_name(sanji_resource, SANJI_REGISTER_WILDCARD_RESOURCE, SANJI_REGISTER_NAME);
	}
	/* 'dependency' model */
	if (!component_is_registered(sanji_component, SANJI_DEPENDENCY_NAME)) {
		component_add_node(sanji_component, SANJI_DEPENDENCY_NAME, "This is dependency model.", SANJI_RESOURCE_DEPENDENCY_TOPIC, "model", NULL, 0, 10, 0);
		resource_append_component_by_name(sanji_resource, SANJI_RESOURCE_DEPENDENCY_RESOURCE, SANJI_DEPENDENCY_NAME);
		resource_append_component_by_name(sanji_resource, SANJI_RESOURCE_DEPENDENCY_WILDCARD_RESOURCE, SANJI_DEPENDENCY_NAME);
	}
}

void sanji_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	fprintf(stderr, "SANJI: log_callback: %s\n", str);
}


/*
 * ##########################
 * MAIN FUNCTION
 * ##########################
 */
int main(int argc, char *argv[])
{
	int rc;

	/* init configs */
	config = sanji_config_init();
	if (!config) {
		fprintf(stderr, "ERROR: Init sanji config failed.\n");
		return 1;
	}

	/* get cmdline options to configs */
	if (sanji_get_cmdline_options(argc, argv, config)) {
		sanji_print_usage();
		sanji_config_free(config);
		return 1;
	}

	/* load config file */
	if (sanji_load_config_file(config)) {
		fprintf(stderr, "WARNING: Load config file '%s' failed\n", config->config_file);
	}

	/* validate configs */
	if (sanji_validate_configs(config)) {
		sanji_config_free(config);
		return 1;
	}

	/* dump configs */
#if (defined DEBUG) || (defined VERBOSE)
	sanji_config_dump(config);
#endif

	/* init userdata */
	ud = sanji_userdata_init();
	if (!ud) {
		fprintf(stderr, "ERROR: Init sanji user data failed.\n");
		sanji_config_free(config);
		return 1;
	}

	/* setup signal handler */
	signal(SIGINT, sanji_signal_quit);
	signal(SIGTERM, sanji_signal_quit);
	signal(SIGUSR1, sanji_signal_dump);

	/* daemonize */
	if (!config->foreground) {
		if (daemonize_process(1)) {
			fprintf(stderr, "ERROR: Failed to deamonize.\n");
			sanji_userdata_free(ud);
			sanji_config_free(config);
			return 1;
		}
	}

	/* init mosquitto library */
	mosquitto_lib_init();

	/* init mosquitto */
	mosq = mosquitto_new(ud->client_id, config->clean_session, ud);
	if (!mosq) {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		mosquitto_lib_cleanup();
		sanji_userdata_free(ud);
		sanji_config_free(config);
		return 1;
	}

	/* setup mosquitto */
	if (config->mosq_debug) {
		mosquitto_log_callback_set(mosq, sanji_log_callback);
	}
	if (config->username && mosquitto_username_pw_set(mosq, config->username, config->password)) {
		fprintf(stderr, "ERROR: Problem setting username and password.\n");
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();
		sanji_userdata_free(ud);
		sanji_config_free(config);
		return 1;
	}
	mosquitto_connect_callback_set(mosq, sanji_connect_callback);
	mosquitto_message_callback_set(mosq, sanji_message_callback);
	mosquitto_subscribe_callback_set(mosq, sanji_subscribe_callback);

	/* connect mosquitto */
	do {
		rc = mosquitto_connect(mosq, config->host, config->port, config->keepalive);
		if (rc) {
			if (rc == MOSQ_ERR_INVAL) {
				fprintf(stderr, "ERROR: Unable to connect (%d: %s).\n", rc, mosquitto_strerror(rc));
				sanji_run = 0;
			} else {
				fprintf(stderr, "ERROR: %s\n", strerror(errno));
				if (errno == EINVAL) sanji_run = 0;
			}

			if (config->retry > 0) config->retry--;
			sleep(1);

		} else {
			break;
		}
	} while (sanji_run && (config->retry > 0 || config->retry < 0));
	if (rc) {
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();
		sanji_userdata_free(ud);
		sanji_config_free(config);
		return rc;
	}

	/*
	 * loop mosquitto,
	 * it use select() to call back the callback-function which defined above.
	 */
	while (sanji_run) {
		rc = mosquitto_loop(mosq, config->refresh_interval, 1);

		/*  refresh ttl for each session */
		sanji_refresh_session();

		/* dump component, resource and session if we got SIGUSR1 */
		if (sanji_dump) {
			sanji_dump_info();
			sanji_dump = 0;
		}

		if (sanji_run && rc) {
			fprintf(stderr, "SANJI: reconnect to server\n");
			mosquitto_reconnect(mosq);
			sleep(1);
		}
	}

	/* clear mosquitto */
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	/* clear sanji */
	sanji_userdata_free(ud);
	sanji_config_free(config);
	/* clear sanji controller objects */
	if (sanji_resource) resource_free(sanji_resource);
	if (sanji_component) component_free(sanji_component);
	if (sanji_session) session_free(sanji_session);

	return 0;
}

