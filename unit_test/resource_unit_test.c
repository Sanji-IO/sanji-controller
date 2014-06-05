#include "resource.h"

int main() 
{
	struct resource *resource = NULL;
	struct resource *node = NULL;

	/* init */
	resource = resource_init();

	/* add node */
	resource_add_node(resource, "n_aaa", "s_aaa", 1, 0);
	resource_add_node(resource, "n_bbb", NULL, 0, 0);

	/* display, delete, add node */
	DEBUG_PRINT();
	resource_display(resource);

	resource_delete_first_name(resource, "n_aaa");
	DEBUG_PRINT();
	resource_display(resource);

	resource_delete_first_name(resource, "n_aaa");
	DEBUG_PRINT();
	resource_display(resource);

	resource_add_node(resource, "n_ccc", "s_ccc", 1, 0);
	DEBUG_PRINT();
	resource_display(resource);

	/* add node for subscribed component */
	char subscribed_component[5][COMPONENT_NAME_LEN];
	memset(subscribed_component, '\0',  sizeof(subscribed_component));
	strcpy(&subscribed_component[0][0], "s_ddd_1");
	strcpy(&subscribed_component[1][0], "s_ddd_2");
	strcpy(&subscribed_component[3][0], "s_ddd_4");
	strcpy(&subscribed_component[4][0], "s_ddd_5");
	resource_add_node(resource, "n_ddd", (char *)subscribed_component, 5, 0);
	DEBUG_PRINT();
	resource_display(resource);

	/* lookup method */
	char m1[RESOURCE_METHOD_LEN] = "POST";
	char m2[RESOURCE_METHOD_LEN] = "GET";
	char m3[RESOURCE_METHOD_LEN] = "PUT";
	char m4[RESOURCE_METHOD_LEN] = "DELETE";
	char m5[RESOURCE_METHOD_LEN] = "POS";
	char m6[RESOURCE_METHOD_LEN] = "PO";
	char m7[RESOURCE_METHOD_LEN] = "PPOST";
	char m8[RESOURCE_METHOD_LEN] = "PPPOST";
	char m9[RESOURCE_METHOD_LEN] = "POSTT";
	char m10[RESOURCE_METHOD_LEN] = "POSTTT";

	fprintf(stderr, "POST(%d)\n",	resource_lookup_method(m1));
	fprintf(stderr, "GET(%d)\n",	resource_lookup_method(m2));
	fprintf(stderr, "UPDATE(%d)\n",resource_lookup_method(m3));
	fprintf(stderr, "DELETE(%d)\n",resource_lookup_method(m4));
	fprintf(stderr, "POS(%d)\n", 	resource_lookup_method(m5));
	fprintf(stderr, "PO(%d)\n", 	resource_lookup_method(m6));
	fprintf(stderr, "PPOST(%d)\n",	resource_lookup_method(m7));
	fprintf(stderr, "PPPOST(%d)\n",	resource_lookup_method(m8));
	fprintf(stderr, "POSTT(%d)\n",	resource_lookup_method(m9));
	fprintf(stderr, "POSTTT(%d)\n",	resource_lookup_method(m10));

	/* is registered */
	fprintf(stderr, "n_ddd is registerd? (%d)\n", resource_is_registered(resource, "n_ddd"));
	fprintf(stderr, "n_nnn is registerd? (%d)\n", resource_is_registered(resource, "n_nnn"));

	/* append component by name */
	resource_append_component_by_name(resource, "n_ddd", "s_ddd_append1");
	resource_append_component_by_name(resource, "n_ddd", "s_ddd_append2");
	resource_append_component_by_name(resource, "n_eee", "s_eee_append1");
	resource_display(resource);

	/* remove component by name */
	resource_remove_component_by_name(resource, "n_ccc", "s_ccc");
	resource_remove_component_by_name(resource, "n_ddd", "s_ddd_4");
	resource_remove_component_by_name(resource, "n_ddd", "s_ddd_6");
	resource_remove_component_by_name(resource, "n_fff", "s_fff");
	resource_display(resource);

	/* lookup node by name */
	node = resource_lookup_node_by_name(resource, "n_aaa");
	if (node) DEBUG_PRINT("find node name(%s)", node->name);
	node = resource_lookup_node_by_name(resource, "n_bbb");
	if (node) DEBUG_PRINT("find node name(%s)", node->name);
	node = resource_lookup_node_by_name(resource, "n_ccc");
	if (node) DEBUG_PRINT("find node name(%s)", node->name);
	node = resource_lookup_node_by_name(resource, "n_ddd");
	if (node) DEBUG_PRINT("find node name(%s)", node->name);
	node = resource_lookup_node_by_name(resource, "n_eee");
	if (node) DEBUG_PRINT("find node name(%s)", node->name);
	node = resource_lookup_node_by_name(resource, "n_fff");
	if (node) DEBUG_PRINT("find node id(%s)", node->name);

	/* lock by name */
	int ret;
	ret = resource_lock_by_name(resource, "n_aaa");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = resource_lock_by_name(resource, "n_bbb");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = resource_lock_by_name(resource, "n_ccc");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = resource_lock_by_name(resource, "n_ddd");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = resource_lock_by_name(resource, "n_eee");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = resource_lock_by_name(resource, "n_fff");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = resource_lock_by_name(resource, "n_aaa");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = resource_lock_by_name(resource, "n_bbb");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = resource_lock_by_name(resource, "n_ccc");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = resource_lock_by_name(resource, "n_ddd");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = resource_lock_by_name(resource, "n_eee");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = resource_lock_by_name(resource, "n_fff");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	resource_display(resource);

	/* unlock by name */
	ret = resource_unlock_by_name(resource, "n_aaa");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = resource_unlock_by_name(resource, "n_bbb");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = resource_unlock_by_name(resource, "n_ccc");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = resource_unlock_by_name(resource, "n_ddd");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = resource_unlock_by_name(resource, "n_eee");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = resource_unlock_by_name(resource, "n_fff");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = resource_unlock_by_name(resource, "n_aaa");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = resource_unlock_by_name(resource, "n_bbb");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = resource_unlock_by_name(resource, "n_ccc");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = resource_unlock_by_name(resource, "n_ddd");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = resource_unlock_by_name(resource, "n_eee");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = resource_unlock_by_name(resource, "n_fff");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	resource_display(resource);

	/* free */
	resource_free(resource);




	/* get name by component */
	DEBUG_PRINT();
	resource = resource_init();
	resource_add_node(resource, "n_aaa", "s_aaa", 1, 0);
	resource_add_node(resource, "n_bbb", "s_aaa", 1, 0);
	resource_add_node(resource, "n_ccc", "s_ccc", 1, 0);
	resource_add_node(resource, "n_ddd", NULL, 0, 0);
	resource_display(resource);

	char *names = NULL;
	unsigned int names_count;
	int i;
	names = resource_get_names_by_component(resource, "s_aaa", &names_count);
	if (names_count > 0) {
		for (i = 0; i < names_count; i++) DEBUG_PRINT("get names(%s) by component(%s)", names + i * RESOURCE_NAME_LEN, "s_aaa");
		free(names);
	}
	names = resource_get_names_by_component(resource, "s_bbb", &names_count);
	if (names_count > 0) {
		for (i = 0; i < names_count; i++) DEBUG_PRINT("get names(%s) by component(%s)", names + i * RESOURCE_NAME_LEN, "s_bbb");
		free(names);
	}
	names = resource_get_names_by_component(resource, "s_ccc", &names_count);
	if (names_count > 0) {
		for (i = 0; i < names_count; i++) DEBUG_PRINT("get names(%s) by component(%s)", names + i * RESOURCE_NAME_LEN, "s_ccc");
		free(names);
	}
	names = resource_get_names_by_component(resource, "s_ddd", &names_count);
	if (names_count > 0) {
		for (i = 0; i < names_count; i++) DEBUG_PRINT("get names(%s) by component(%s)", names + i * RESOURCE_NAME_LEN, "s_ccc");
		free(names);
	}

	/* free */
	resource_free(resource);

	return 0;
}
