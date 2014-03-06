#include <jansson.h>
#include "session.h"

#define JSON_FILE "./log.json"

int main() 
{
	struct session *session = NULL;
	struct session *node = NULL;
	int i, j;
	/* result chain */
	json_error_t error;
	json_t *result_chain2 = NULL; 
	json_t *result_chain3 = NULL; 
	//char *str = NULL;

	/* model chain */
	struct model_chain *model_chain = NULL;
	struct model_chain *tmp = NULL;
	int model_chain_count = 3;

	/* view chain */
	char *view_chain = NULL;
	int view_chain_count = 4;



	/* result_chain */
	result_chain2 = json_load_file(JSON_FILE, 0, &error);
	if (!result_chain2) {
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		return 1;
	}
	//fprintf(stderr, "Start dump:\n");
	//str = json_dumps(result_chain2, JSON_INDENT(4));
	//fprintf(stderr, "%s\n", str);
	//free(str);
	result_chain3 = json_deep_copy(result_chain2);
	//fprintf(stderr, "Start dump:\n");
	//str = json_dumps(result_chain3, JSON_INDENT(4));
	//fprintf(stderr, "%s\n", str);
	//free(str);


	/* model_chain */
	model_chain = (struct model_chain *)malloc(model_chain_count * sizeof(struct model_chain));
	memset(model_chain, 0, model_chain_count * sizeof(struct model_chain));
	for (i = 0; i < model_chain_count; i++) {
		tmp = &model_chain[i];
		tmp->count = i;
		tmp->ttls = (int *)malloc(tmp->count * sizeof(int));
		tmp->models = (char *)malloc(tmp->count * COMPONENT_NAME_LEN);
		memset(tmp->models, '\0', tmp->count * COMPONENT_NAME_LEN);
		for (j = 0; j < tmp->count; j++) {
			strncpy(tmp->models + j * COMPONENT_NAME_LEN, "model name", COMPONENT_NAME_LEN);
			tmp->ttls[j] = i * 10 + j;
		}
	}

	/* view_chain */
	view_chain = (char *)malloc(view_chain_count * COMPONENT_NAME_LEN);
	memset(view_chain, '\0', view_chain_count * COMPONENT_NAME_LEN);
	for (i = 0; i < view_chain_count; i++) {
		strncpy(view_chain + i * COMPONENT_NAME_LEN, "view name", COMPONENT_NAME_LEN);
	}




	/* init session */
	session = session_init();

	session_add_node(session, 1, -1, "r_aaa", NULL, NULL, 0, NULL, 0);
	session_add_node(session, 2, 0, "r_bbb", result_chain2, NULL, 0, NULL, 0);
	session_add_node(session, 3, 1, "r_ccc", result_chain3, model_chain, model_chain_count, view_chain, view_chain_count);


	DEBUG_PRINT();
	session_display(session);

	session_delete_first_resource(session, "r_aaa");
	DEBUG_PRINT();
	session_display(session);

	session_delete_first_resource(session, "r_aaa");
	DEBUG_PRINT();
	session_display(session);

	session_delete_first_id(session, 1);
	DEBUG_PRINT();
	session_display(session);

	session_delete_first_id(session, 2);
	DEBUG_PRINT();
	session_display(session);

	session_delete_first_id(session, 2);
	DEBUG_PRINT();
	session_display(session);

	session_add_node(session, 4, 2, "r_ddd", NULL, NULL, 0, NULL, 0);
	DEBUG_PRINT();
	session_display(session);

	/* lookup node by id */
	node = session_lookup_node_by_id(session, 1);
	if (node) DEBUG_PRINT("find node id(%d)", node->id);
	node = session_lookup_node_by_id(session, 2);
	if (node) DEBUG_PRINT("find node id(%d)", node->id);
	node = session_lookup_node_by_id(session, 3);
	if (node) DEBUG_PRINT("find node id(%d)", node->id);
	node = session_lookup_node_by_id(session, 4);
	if (node) DEBUG_PRINT("find node id(%d)", node->id);
	node = session_lookup_node_by_id(session, 5);
	if (node) DEBUG_PRINT("find node id(%d)", node->id);

	session_free(session);

	return 0;
}
