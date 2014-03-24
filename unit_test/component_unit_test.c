#include "component.h"

int main() 
{
	struct component *component = NULL;
	struct component *node = NULL;
	int i;

	component = component_init();

	component_display(component);
	component_add_node(component, "n_aaa", "d_aaa", "t_aaa", "r_aaa", "h_aaa", 1, 111, 0);
	component_add_node(component, "n_bbb", NULL, "t_bbb", "r_bbb", "h_bbb", 1, 222, 0);
	component_add_node(component, "n_ccc", NULL, "t_ccc", "r_ccc", NULL, 0, 333, 0);

	DEBUG_PRINT();
	component_display(component);

	component_delete_first_name(component, "n_aaa");
	DEBUG_PRINT();
	component_display(component);

	component_delete_first_name(component, "n_aaa");
	DEBUG_PRINT();
	component_display(component);

	component_add_node(component, "n_ddd", "d_ddd", "t_ddd", "r_ddd", "h_ddd", 1, 444, 0);
	DEBUG_PRINT();
	component_display(component);

	char hook[4][COMPONENT_NAME_LEN];
	memset(hook, '\0',  sizeof(hook));
	strcpy(&hook[0][0], "h_eee_1");
	strcpy(&hook[1][0], "h_eee_2");
	strcpy(&hook[3][0], "h_eee_4");
	component_add_node(component, "n_eee", "d_eee", "t_eee", "r_eee", (char *)hook, 4, 555, 0);
	DEBUG_PRINT();
	component_display(component);

	/* is registered */
	fprintf(stderr, "n_aaa is registerd? (%d)\n", component_is_registered(component, "n_aaa"));
	fprintf(stderr, "n_bbb is registerd? (%d)\n", component_is_registered(component, "n_bbb"));

	/* is unique tunnel */
	fprintf(stderr, "t_aaa is unique? (%d)\n", component_is_unique_tunnel(component, "t_aaa"));
	fprintf(stderr, "t_bbb is unique? (%d)\n", component_is_unique_tunnel(component, "t_bbb"));
	fprintf(stderr, "t_ccc is unique? (%d)\n", component_is_unique_tunnel(component, "t_ccc"));
	fprintf(stderr, "t_ddd is unique? (%d)\n", component_is_unique_tunnel(component, "t_ddd"));

	/* append component by name */
	component_append_hook_by_name(component, "n_ccc", "h_ccc_append1");
	component_append_hook_by_name(component, "n_ccc", "h_ccc_append2");
	component_append_hook_by_name(component, "n_eee", "h_eee_append1");
	component_display(component);

	DEBUG_PRINT();
	/* remove component by name */
	component_remove_hook_by_name(component, "n_ccc", "h_ccc");
	component_remove_hook_by_name(component, "n_ddd", "h_ddd");
	component_remove_hook_by_name(component, "n_eee", "h_eee_2");
	component_remove_hook_by_name(component, "n_eee", "h_eee_3");
	component_remove_hook_by_name(component, "n_fff", "h_fff");
	component_display(component);

	/* lookup node by name */
	node = component_lookup_node_by_name(component, "n_aaa");
	if (node) DEBUG_PRINT("find node name(%s)", node->name);
	node = component_lookup_node_by_name(component, "n_bbb");
	if (node) DEBUG_PRINT("find node name(%s)", node->name);
	node = component_lookup_node_by_name(component, "n_ccc");
	if (node) DEBUG_PRINT("find node name(%s)", node->name);
	node = component_lookup_node_by_name(component, "n_ddd");
	if (node) DEBUG_PRINT("find node name(%s)", node->name);
	node = component_lookup_node_by_name(component, "n_eee");
	if (node) DEBUG_PRINT("find node name(%s)", node->name);
	node = component_lookup_node_by_name(component, "n_fff");
	if (node) DEBUG_PRINT("find node id(%s)", node->name);

	/* lock by name */
	int ret;
	ret = component_lock_by_name(component, "n_aaa");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = component_lock_by_name(component, "n_bbb");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = component_lock_by_name(component, "n_ccc");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = component_lock_by_name(component, "n_ddd");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = component_lock_by_name(component, "n_eee");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = component_lock_by_name(component, "n_fff");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = component_lock_by_name(component, "n_aaa");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = component_lock_by_name(component, "n_bbb");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = component_lock_by_name(component, "n_ccc");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = component_lock_by_name(component, "n_ddd");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = component_lock_by_name(component, "n_eee");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	ret = component_lock_by_name(component, "n_fff");
	DEBUG_PRINT("lock by name return code(%d)", ret);
	component_display(component);

	/* unlock by name */
	ret = component_unlock_by_name(component, "n_aaa");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = component_unlock_by_name(component, "n_bbb");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = component_unlock_by_name(component, "n_ccc");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = component_unlock_by_name(component, "n_ddd");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = component_unlock_by_name(component, "n_eee");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = component_unlock_by_name(component, "n_fff");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = component_unlock_by_name(component, "n_aaa");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = component_unlock_by_name(component, "n_bbb");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = component_unlock_by_name(component, "n_ccc");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = component_unlock_by_name(component, "n_ddd");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = component_unlock_by_name(component, "n_eee");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	ret = component_unlock_by_name(component, "n_fff");
	DEBUG_PRINT("unlock by name return code(%d)", ret);
	component_display(component);



	/* free */
	component_free(component);







	/* get name by hook */
	component = component_init();
	component_add_node(component, "n_aaa", "d_aaa", "t_aaa", "r_aaa", NULL, 0, 111, 0);
	component_add_node(component, "n_bbb", NULL, "t_bbb", "r_bbb", "n_aaa", 1, 222, 0);
	component_add_node(component, "n_ccc", NULL, "t_ccc", "r_ccc", "n_aaa", 1, 333, 0);
	component_display(component);

	char *names = NULL;
	unsigned int names_count;
	names = component_get_names_by_hook(component, "n_aaa", &names_count);
	if (names_count > 0) {
		for (i = 0; i < names_count; i++) DEBUG_PRINT("get names(%s) by hook(%s)", names + i * COMPONENT_NAME_LEN, "n_aaa");
		free(names);
	}
	names = component_get_names_by_hook(component, "n_bbb", &names_count);
	if (names_count > 0) {
		for (i = 0; i < names_count; i++) DEBUG_PRINT("get names(%s) by hook(%s)", names + i * COMPONENT_NAME_LEN, "n_bbb");
		free(names);
	}
	names = component_get_names_by_hook(component, "n_ccc", &names_count);
	if (names_count > 0) {
		for (i = 0; i < names_count; i++) DEBUG_PRINT("get names(%s) by hook(%s)", names + i * COMPONENT_NAME_LEN, "n_ccc");
		free(names);
	}


	/* free */
	component_free(component);

	return 0;
}
