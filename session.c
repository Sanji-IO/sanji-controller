#include "session.h"

struct session *session_init()
{
	struct session *head = NULL;

	head = (struct session *)malloc(sizeof(struct session));
	if  (!head) {
		fprintf(stderr, "Error: out of memory.\n");
		return NULL;
	}

	/* initialize list */
	INIT_LIST_HEAD(&head->list);

	return head;
}

struct session *session_create_node(
		unsigned int id,
		int method,
		char *resource,
		json_t *result_chain,
		struct model_chain *model_chain,
		unsigned int model_chain_count,
		char *view_chain,
		unsigned int view_chain_count)
{
	struct session *node = NULL;
	
	node = (struct session *)malloc(sizeof(struct session));
	if  (!node) {
		fprintf(stderr, "Error: add node failed, out of memory.\n");
		return NULL;
	}
	memset(node, 0, sizeof(struct session));

	INIT_LIST_HEAD(&node->list);
	node->id = id;
	node->method = method;
	if (resource) strncpy(node->resource, resource, RESOURCE_NAME_LEN - 1);
	node->resource[RESOURCE_NAME_LEN - 1] = '\0';
	if (result_chain) node->result_chain = result_chain;
	if (model_chain) node->model_chain = model_chain;
	node->model_chain_count = model_chain_count;
	if (view_chain) node->view_chain = view_chain;
	node->view_chain_count = view_chain_count;
	node->curr_step = 0;
	node->curr_wait = 0;

	return node;
}

int session_add(struct session *head, struct session *node)
{
	if (!head || !node) {
		fprintf(stderr, "Error: add node to head failed\n");
		return 1;
	}

	list_add_tail(&node->list, &head->list);

	return 0;
}

int session_add_node(
		struct session *head,
		unsigned int id,
		int method,
		char *resource,
		json_t *result_chain,
		struct model_chain *model_chain,
		unsigned int model_chain_count,
		char *view_chain,
		unsigned int view_chain_count)

{
	struct session *node = NULL;

	if (!head) {
		fprintf(stderr, "Error: add node failed, list head is empty\n");
		return 1;
	}
	
	node = session_create_node(id, method, resource, result_chain, model_chain, model_chain_count, view_chain, view_chain_count);
	if  (!node) {
		fprintf(stderr, "Error: add node failed, out of memory.\n");
		return 1;
	}

	return session_add(head, node);
}

struct session *session_lookup_node_by_id(struct session *head, unsigned int id)
{
	struct session *curr = NULL;

	if (head) {
		list_for_each_entry(curr, &head->list, list) {
			if (curr->id == id) return curr;
		}
	}

	return NULL;
}

void session_display_model_chain(struct model_chain *model_chain, unsigned int model_chain_count)
{
	int i, j;

	if (model_chain) {
		/* display model chain */
		fprintf(stderr, "\tmodel_chain:\n");
		for (i = 0; i < model_chain_count; i++) {
			fprintf(stderr, "\tmodel_chain(%d)\n", i + 1);
			for (j = 0; j < (&model_chain[i])->count; j++) {
				fprintf(stderr, "\t\tmodels(%s)\n", (&model_chain[i])->models + j * COMPONENT_NAME_LEN);
				fprintf(stderr, "\t\tttls(%d)\n", (&model_chain[i])->ttls[j]);
			}
			fprintf(stderr, "\t\tcount(%d)\n", (&model_chain[i])->count);
		}
	}
}

void session_display_view_chain(char *view_chain, unsigned int view_chain_count)
{
	int i;

	if (view_chain) {
		/* display view chain */
		for (i = 0; i < view_chain_count; i++) {
			fprintf(stderr, "\tview_chain(%d)(%s)\n", i + 1, view_chain + i * COMPONENT_NAME_LEN);
		}
	}
}

void session_display(struct session *head)
{
	struct session *curr = NULL;
	char *dump_result = NULL;
	int i = 1;

	if (head) {
		list_for_each_entry(curr, &head->list, list) {
			fprintf(stderr, "session node(%d):\n", i++);
			fprintf(stderr, "\tid(%u)\n", curr->id);
			fprintf(stderr, "\tmethod(%d)\n", curr->method);
			fprintf(stderr, "\tresource(%s)\n", curr->resource);
			/* display result chain */
			fprintf(stderr, "\tresult_chain:\n");
			if (curr->result_chain) {
				dump_result = json_dumps(curr->result_chain, JSON_INDENT(4));
				if (dump_result) {
					fprintf(stderr, "%s\n", dump_result);
					free(dump_result);
				}
			}
			/* display model chain */
			session_display_model_chain(curr->model_chain, curr->model_chain_count);
			fprintf(stderr, "\tmodel_chain_count(%d)\n", curr->model_chain_count);
			/* display view chain */
			session_display_view_chain(curr->view_chain, curr->view_chain_count);
			fprintf(stderr, "\tview_chain_count(%d)\n", curr->view_chain_count);
			fprintf(stderr, "\tcurr_step(%d)\n", curr->curr_step);
			fprintf(stderr, "\tcurr_wait(%d)\n", curr->curr_wait);
		}
	}
}

void session_free(struct session *head)
{
	struct session *curr = NULL;

	if (head) {
		while (!list_empty(&head->list)) {
			curr = list_entry(head->list.next, struct session, list);
			list_del(&curr->list);
			if (curr->result_chain) json_decref(curr->result_chain);
			if (curr->model_chain) session_free_model_chain(curr->model_chain, curr->model_chain_count);
			if (curr->view_chain) free(curr->view_chain);
			free(curr);
		}
		free(head);
	}
}

void session_free_model_chain(struct model_chain *model_chain, unsigned int model_chain_count)
{
	int i;

	if (model_chain) {
		for (i = 0; i < model_chain_count; i++) {
			if ((&model_chain[i])->models) free((&model_chain[i])->models);
			if ((&model_chain[i])->ttls) free((&model_chain[i])->ttls);
		}
		free(model_chain);
	}
}

int session_delete_first_resource(struct session *head, char *resource)
{
	struct session *curr = NULL;

	if (!head || !resource) {
		fprintf(stderr, "Error: list head or resource which wanted deleted is empty.\n");
		return 1;
	}

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->resource, resource)) {
			list_del(&curr->list);
			if (curr->result_chain) json_decref(curr->result_chain);
			if (curr->model_chain) session_free_model_chain(curr->model_chain, curr->model_chain_count);
			if (curr->view_chain) free(curr->view_chain);
			free(curr);
			return 1;
		}
	}

	return 0;
}

int session_delete_first_id(struct session *head, unsigned int id)
{
	struct session *curr = NULL;

	if (!head) {
		fprintf(stderr, "Error: list head which wanted deleted is empty.\n");
		return 1;
	}

	list_for_each_entry(curr, &head->list, list) {
		if (curr->id == id) {
			list_del(&curr->list);
			if (curr->result_chain) json_decref(curr->result_chain);
			if (curr->model_chain) session_free_model_chain(curr->model_chain, curr->model_chain_count);
			if (curr->view_chain) free(curr->view_chain);
			free(curr);
			return 1;
		}
	}

	return 0;
}

int session_is_inflight(struct session *head, unsigned int id)
{
	struct session *curr = NULL;

	if (!head) {
		fprintf(stderr, "Error: list head or name is empty.\n");
		return SANJI_DATA_ERROR;
	}

	list_for_each_entry(curr, &head->list, list) {
		if (curr->id == id) {
			return 1;
		}
	}

	return 0;
}

int session_node_unlock_by_step(struct session *node, struct resource *resource_list, struct component *component_list, unsigned int step)
{
	struct model_chain *model_chain = NULL;
	char *model = NULL;
	int ret;
	int i, j;

	if (!node || !resource_list || !component_list || step < 0) {
		fprintf(stderr, "Error: session node is empty.\n");
		return SANJI_DATA_ERROR;
	}

	if (!step) {
		/* unlock resource */
		ret = resource_unlock_by_name(resource_list, node->resource);
		if (ret) return ret;

		/* unlock all models of all steps */
		for (i = 0; i < node->model_chain_count; i++) {
			model_chain = &node->model_chain[i];
			for (j = 0; j < model_chain->count; j++) {
				model = model_chain->models + j * COMPONENT_NAME_LEN;
				ret = component_unlock_by_name(component_list, model);
				if (ret) return ret;
			}
		}
	} else if (step <= node->model_chain_count) {
		/* unlock all models of this step */
		model_chain = &node->model_chain[step - 1];
		for (j = 0; j < model_chain->count; j++) {
			model = model_chain->models + j * COMPONENT_NAME_LEN;
			ret = component_unlock_by_name(component_list, model);
			if (ret) return ret;
		}
	}

	return SANJI_SUCCESS;
}

int session_node_lock_by_step(struct session *node, struct resource *resource_list, struct component *component_list, unsigned int step)
{
	struct model_chain *model_chain = NULL;
	char *model = NULL;
	int ret;
	int i, j;

	if (!node || !resource_list || !component_list || step < 0) {
		fprintf(stderr, "Error: session node is empty.\n");
		return SANJI_DATA_ERROR;
	}

	if (!step) {
		/* lock resource */
		ret = resource_lock_by_name(resource_list, node->resource);
		if (ret) return ret;

		/* lock all component of all steps */
		for (i = 0; i < node->model_chain_count; i++) {
			model_chain = &node->model_chain[i];
			for (j = 0; j < model_chain->count; j++) {
				model = model_chain->models + j * COMPONENT_NAME_LEN;
				ret = component_lock_by_name(component_list, model);
				if (ret) return ret;
			}
		}
	} else if (step <= node->model_chain_count) {
		/* lock all component of this step */
		model_chain = &node->model_chain[step - 1];
		for (j = 0; j < model_chain->count; j++) {
			model = model_chain->models + j * COMPONENT_NAME_LEN;
			ret = component_lock_by_name(component_list, model);
			if (ret) return ret;
		}
	}

	return SANJI_SUCCESS;
}

void session_node_update_wait(struct session *node)
{
	struct model_chain *model_chain = NULL;

	if (node) {
		model_chain = &node->model_chain[node->curr_step - 1];
		node->curr_wait = model_chain->count;
	}
}

void session_node_decref_wait(struct session *node, struct session *session_list, struct resource *resource_list, struct component *component_list, struct mosquitto *mosq, void *obj, json_t *root)
{
	if (!node) return;

	node->curr_wait--;
	DEBUG_PRINT("session(%d) get response at step(%d/%d), left wait(%d).", node->id, node->curr_step, node->model_chain_count, node->curr_wait);

	/* move to next step */
	if (node->curr_wait <= 0) {
		session_step(node, session_list, resource_list, component_list, mosq, obj, root);
	}
}

int session_step_stop(
		struct session *node, 
		struct session *session_list,
		struct resource *resource_list, 
		struct component *component_list, 
		struct mosquitto *mosq, 
		void *obj, 
		json_t *root)
{
	struct sanji_userdata *ud = NULL;
	char *view = NULL;
	char *tunnel = NULL;
	json_t *result = NULL;
	json_t *data = NULL;
	json_t *result_data = NULL;
	json_t *root_data = NULL;
	json_t *response_root = NULL;
	char *packet_context = NULL;
	int packet_context_len;
	int i;

	/* dump packet context from json object 'root' and node->result_chain */
	if (node->curr_step > node->model_chain_count) {
		/* sesion success: append/update all 'data' */
		DEBUG_PRINT("session(%d) succeed with total step(%d).", node->id, node->model_chain_count);
		if (node->result_chain && json_is_array(node->result_chain)) {
			for (i = 0; i < json_array_size(node->result_chain); i++) {
				result = json_array_get(node->result_chain, i);
				if (result && json_is_object(result)) {
					result_data = json_object_get(result, "data");
					root_data = json_object_get(root, "data");
					json_object_update(root_data, result_data);
				}
			}
		}
		response_root = root;

	} else if (!root) {
		/* sesion fail due to ttl: add 'code', 'id' */
		DEBUG_PRINT("session(%d) failed due to ttl expired.", node->id);

		/* must free 'response_root' later */
		response_root = json_object();
		json_object_set_new(response_root, "code", json_integer(HTTP_REQUEST_TIME_OUT));
		json_object_set_new(response_root, "id", json_integer(node->id));
		data = json_object();
		json_object_set_new(data, "message", json_string("session failed due to ttl expired ."));
		json_object_set_new(response_root, "data", data);

	} else {
		DEBUG_PRINT("session(%d) failed at step(%d/%d), at wait(%d).", node->id, node->curr_step, node->model_chain_count, node->curr_wait);
		/* sesion fail: merge return 'code', write 'log' and 'message' */
#if 0 // Merge result
		json_object_del(root, "data");
		data = json_object();
		json_object_set_new(data, "message", json_string("failed to operate the resource, please see log for more details."));
		json_object_set(data, "log", node->result_chain);
		json_object_set_new(root, "data", data);
#else
#endif
		response_root = root;
	}

	/* dump response context */
	if (node->view_chain_count) {
		packet_context = json_dumps(response_root, JSON_INDENT(4));
		if (!packet_context) {
			DEBUG_PRINT("Error: out of memory");
			if (!root) json_decref(response_root);
			return SANJI_INTERNAL_ERROR;
		}
		packet_context_len = strlen(packet_context);
	}
	if (!root) json_decref(response_root);

	/* publish to all views */
	ud = (struct sanji_userdata *)obj;
	for (i = 0; i < node->view_chain_count; i++) {
		view = node->view_chain + i * COMPONENT_NAME_LEN;
		tunnel = component_get_tunnel_by_name(component_list, view);
		DEBUG_PRINT("session(%d) sends to view(%s) with tunnel(%s).", node->id, view, tunnel);
		mosquitto_publish(mosq, &ud->mid_sent, tunnel, packet_context_len, packet_context, ud->qos_sent, ud->retain_sent);
	}
	if (packet_context) {
		DEBUG_PRINT("%s", packet_context);
		free(packet_context);
	}

	/* unlock all related resource/models for write-like method */
	if (resource_is_write_like_method(node->method)) {
		session_node_unlock_by_step(node, resource_list, component_list, 0);
	}

	/* free session */
	session_delete_first_id(session_list, node->id);

#if (defined DEBUG) || (defined VERBOSE)
	DEBUG_PRINT("dump sessions:");
	session_display(session_list);
#endif

	return SANJI_SUCCESS;
}

int session_step(
		struct session *node, 
		struct session *session_list, 
		struct resource *resource_list, 
		struct component *component_list, 
		struct mosquitto *mosq, 
		void *obj, 
		json_t *root)
{
	struct sanji_userdata *ud = NULL;
	struct model_chain *model_chain = NULL;
	char *model = NULL;
	char *tunnel = NULL;
	json_t *result = NULL;
	json_t *root_data = NULL;
	json_t *result_data = NULL;
	char *packet_context = NULL;
	int packet_context_len;
	int i;

	if (!node || !resource_list || !component_list || !mosq || !obj || !root) return -1;

	/* move step */
	node->curr_step++;

	/* finish step */
	if (node->curr_step > node->model_chain_count) {
		return session_step_stop(node, session_list, resource_list, component_list, mosq, obj, root);
	}

	/* update number of waited models for this step */
	session_node_update_wait(node);

	/* dump all 'data' from node->result_chain to json object 'root' */
	if (node->curr_step != 1) {
		/* append/update all 'data' */
		if (node->result_chain && json_is_array(node->result_chain)) {
			for (i = 0; i < json_array_size(node->result_chain); i++) {
				result = json_array_get(node->result_chain, i);
				if (result && json_is_object(result)) {
					result_data = json_object_get(result, "data");
					root_data = json_object_get(root, "data");
					json_object_update(root_data, result_data);
				}
			}
		}
		/* remove code */
		json_object_del(root, "code");
	}

	/* dump packet context from json object 'root' */
	packet_context = json_dumps(root, JSON_INDENT(4));
	if (!packet_context) {
		DEBUG_PRINT("Error: out of memory");
		return SANJI_INTERNAL_ERROR;
	}
	packet_context_len = strlen(packet_context);

	/* publish to all hook models */
	ud = (struct sanji_userdata *)obj;
	model_chain = &node->model_chain[node->curr_step - 1];
	for (i = 0; i < node->curr_wait; i++) {
		model = model_chain->models + i * COMPONENT_NAME_LEN;
		tunnel = component_get_tunnel_by_name(component_list, model);
		DEBUG_PRINT("session(%d) on step(%d/%d) sends to model(%s) with tunnel(%s) in wait(%d/%d)", node->id, node->curr_step, node->model_chain_count, model, tunnel, i + 1, node->curr_wait);
		mosquitto_publish(mosq, &ud->mid_sent, tunnel, packet_context_len, packet_context, ud->qos_sent, ud->retain_sent);
	}
	DEBUG_PRINT("%s", packet_context);
	free(packet_context);

	return SANJI_SUCCESS;
}

void session_step_update(struct session *node, 
		struct session *session_list, 
		struct resource *resource_list, 
		struct component *component_list, 
		struct mosquitto *mosq, 
		void *obj, 
		json_t *root, 
		int code, 
		json_t *data)
{
	json_t *result_root = NULL;

	/* append result */
	result_root = json_object();
	json_object_set_new(result_root, "code", json_integer(code));
	json_object_set(result_root, "data", data);
	json_array_append_new(node->result_chain, result_root);

	/* validate return code */
	if (code == SESSION_CODE_OK) {
		session_node_decref_wait(node, session_list, resource_list, component_list, mosq, obj, root);
	} else {
		session_step_stop(node, session_list, resource_list, component_list, mosq, obj, root);
	}
}

void session_decref_ttl(struct session *session_list, struct resource *resource_list, struct component *component_list, struct mosquitto *mosq, void *obj, unsigned int pass_time)
{
	struct session *curr = NULL;
	struct model_chain *model_chain = NULL;
	struct session **flush_list = NULL;
	int flush_count = 0;
	int i;

	if (!session_list) return;

	/* decref ttl */
	list_for_each_entry(curr, &session_list->list, list) {
		model_chain = &curr->model_chain[curr->curr_step - 1];
		for (i = 0; i < model_chain->count; i++) {
			model_chain->ttls[i] -= pass_time;
			if (model_chain->ttls[i] <= 0) {
				DEBUG_PRINT("session(%d) will be flushed due to expired ttl.", curr->id);
				flush_count++;
				flush_list = realloc(flush_list, flush_count * sizeof(struct session *));
				flush_list[flush_count - 1] = curr;
				break;
			}
		}
	}

	/* flush expired session */
	for (i = 0; i < flush_count; i++) {
		session_step_stop(flush_list[i], session_list, resource_list, component_list, mosq, obj, NULL);
	}
	if (flush_list) free(flush_list);

#if (defined DEBUG) || (defined VERBOSE)
	DEBUG_PRINT("dump sessions:");
	session_display(session_list);
#endif

}

