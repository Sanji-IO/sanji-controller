#include "component.h"

struct component *component_init()
{
	struct component *head = NULL;

	head = (struct component *)malloc(sizeof(struct component));
	if  (!head) {
		fprintf(stderr, "Error: out of memory.\n");
		return NULL;
	}

	/* initialize list */
	INIT_LIST_HEAD(&head->list);

	return head;
}

int component_add_node(struct component *head, char *name, char *description, char *tunnel, char *role, char *hook, unsigned int hook_count, int ttl, unsigned int lock)
{
	struct component *node = NULL;

	if (!head) {
		fprintf(stderr, "Error: add node failed, list head is empty\n");
		return 1;
	}
	
	node = (struct component *)malloc(sizeof(struct component));
	if  (!node) {
		fprintf(stderr, "Error: add node failed, out of memory.\n");
		return 1;
	}
	memset(node, 0, sizeof(struct component));

	INIT_LIST_HEAD(&node->list);
	if (name) strncpy(node->name, name, COMPONENT_NAME_LEN - 1);
	node->name[COMPONENT_NAME_LEN - 1] = '\0';
	if (description) strncpy(node->description, description, COMPONENT_DESCRIPTION_LEN - 1);
	node->description[COMPONENT_DESCRIPTION_LEN - 1] = '\0';
	if (tunnel) strncpy(node->tunnel, tunnel, COMPONENT_TUNNEL_LEN - 1);
	node->tunnel[COMPONENT_TUNNEL_LEN - 1] = '\0';
	if (role) strncpy(node->role, role, COMPONENT_ROLE_LEN - 1);
	node->role[COMPONENT_ROLE_LEN - 1] = '\0';
	node->ttl = ttl;
	node->lock = lock;

	node->hook_count = hook_count;
	if (hook) {
		node->hook = malloc(hook_count * COMPONENT_NAME_LEN);
		if  (!node->hook) {
			fprintf(stderr, "Error: add node failed, out of memory.\n");
			free(node);
			return 1;
		}
		memset(node->hook, '\0', hook_count * COMPONENT_NAME_LEN);
		memcpy(node->hook, hook, hook_count * COMPONENT_NAME_LEN);
		node->hook[hook_count * COMPONENT_NAME_LEN - 1] = '\0';
	}

	list_add_tail(&node->list, &head->list);

	return 0;
}

void component_display(struct component *head)
{
	struct component *curr = NULL;
	int i = 1, j;

	if (head) {
		list_for_each_entry(curr, &head->list, list) {
			fprintf(stderr, "component node(%d):\n", i++);
			fprintf(stderr, "\tname(%s)\n", curr->name);
			fprintf(stderr, "\tdescription(%s)\n", curr->description);
			fprintf(stderr, "\ttunnel(%s)\n", curr->tunnel);
			fprintf(stderr, "\trole(%s)\n", curr->role);
			for (j = 0; j < curr->hook_count; j++) {
				fprintf(stderr, "\thook(%s)\n", curr->hook + (j * COMPONENT_NAME_LEN));
			}
			fprintf(stderr, "\thook_count(%d)\n", curr->hook_count);
			fprintf(stderr, "\tttl(%d)\n", curr->ttl);
			fprintf(stderr, "\tlock(%d)\n", curr->lock);
		}
	}
}

void component_free(struct component *head)
{
	struct component *curr = NULL;

	if (head) {
		while (!list_empty(&head->list)) {
			curr = list_entry(head->list.next, struct component, list);
			list_del(&curr->list);
			if (curr->hook) free(curr->hook);
			free(curr);
		}
		free(head);
	}
}

int component_delete_first_name(struct component *head, char *name)
{
	struct component *curr = NULL;

	if (!head || !name) {
		fprintf(stderr, "Error: list head or name which wanted deleted is empty.\n");
		return 1;
	}

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) {
			if (!curr->lock) {
				list_del(&curr->list);
				if (curr->hook) free(curr->hook);
				free(curr);
				return SANJI_SUCCESS;
			} else {
				return SANJI_LOCKED;
			}
		}
	}

	return SANJI_SUCCESS;
}

int component_is_registered(struct component *head, char *name)
{
	struct component *curr = NULL;

	if (!head || !name) {
		fprintf(stderr, "Error: list head or name is empty.\n");
		return SANJI_DATA_ERROR;
	}

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) {
			return 1;
		}
	}

	return 0;
}

int component_is_locked(struct component *head, char *name)
{
	struct component *curr = NULL;

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) {
			return curr->lock;
		}
	}

	return 0;
}

int component_node_is_locked(struct component *node)
{
	return node->lock;
}

int component_is_given_role(struct component *head, char *name, char *role)
{
	struct component *curr = NULL;

	curr = component_lookup_node_by_name(head, name);
	if (curr) return component_node_is_given_role(curr, role);

	return 0;
}

int component_node_is_given_role(struct component *node, char *role)
{
	if (!node || !role) return 0;
	if (!strcmp(node->role, role)) return 1;

	return 0;
}

int component_is_unique_tunnel(struct component *head, char *tunnel)
{
	struct component *curr = NULL;

	if (!head || !tunnel) return 0;

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->tunnel, tunnel)) return 0;
	}

	return 1;
}

int component_append_hook_by_name(struct component *head, char *name, char *hook)
{
	struct component *curr = NULL;
	int is_registered;
	char *tmp = NULL;

	if (!head || !name || !hook) {
		fprintf(stderr, "Error: append hook failed\n");
		return SANJI_DATA_ERROR;
	}

	/* check whether already been registered */
	is_registered = component_is_registered(head, name);
	if (!is_registered || is_registered == -1) {
		return SANJI_DATA_ERROR;
	}
	
	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) {
			if (!curr->lock) {
				curr->lock = 1;
				/*
				 * Reallocate memory.
				 * Note, be careful to use realloc().
				 */
				curr->hook_count++;
				tmp = realloc(curr->hook, curr->hook_count * COMPONENT_NAME_LEN);
				if (!tmp) {
					fprintf(stderr, "Error: append node failed, out of memory.\n");
					return SANJI_DATA_ERROR;
				}
				curr->hook = tmp;
				memset(curr->hook + (curr->hook_count - 1) * COMPONENT_NAME_LEN, '\0', COMPONENT_NAME_LEN);
				memcpy(curr->hook + (curr->hook_count - 1) * COMPONENT_NAME_LEN, hook, COMPONENT_NAME_LEN);
				curr->hook[curr->hook_count * COMPONENT_NAME_LEN - 1] = '\0';
				curr->lock = 0;
				return SANJI_SUCCESS;
			} else {
				return SANJI_LOCKED;
			}
		}
	}

	return SANJI_SUCCESS;
}

int component_remove_hook_by_name(struct component *head, char *name, char *hook)
{
	struct component *curr = NULL;
	int is_registered;
	int is_finded = 0;
	int hook_index = -1;
	char *tmp = NULL;
	int i;

	if (!head || !name || !hook) {
		fprintf(stderr, "Error: remove hook failed\n");
		return SANJI_DATA_ERROR;
	}

	/* check whether already been registered */
	is_registered = component_is_registered(head, name);
	if (is_registered == -1) {
		return SANJI_DATA_ERROR;
	} else if (!is_registered) {
		return SANJI_SUCCESS;;
	}
	
	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) {
			if (!curr->lock) {
				curr->lock = 1;

				/* check if hook_count is already '0' or '1', just free it. */
				if (!curr->hook_count || (curr->hook_count == 1)) {
					list_del(&curr->list);
					if (curr->hook)
						free(curr->hook);
					free(curr);
					return SANJI_SUCCESS;
				}

				/* find the index of subscribed component */
				for (i = 0; i < curr->hook_count; i++) {
					if (!strncmp(curr->hook + i * COMPONENT_NAME_LEN, hook, COMPONENT_NAME_LEN)) {
						is_finded = 1;
						hook_index = i;
						break;
					}
				}

				if (!is_finded) {
					curr->lock = 0;
					return SANJI_SUCCESS;
				}
				curr->hook_count--;

				/* cascade subscribed component */
				tmp = malloc(curr->hook_count * COMPONENT_NAME_LEN);
				if (!tmp) {
					fprintf(stderr, "Error: remove node failed, out of memory.\n");
					curr->lock = 0;
					return SANJI_DATA_ERROR;
				}
				memset(tmp, '\0', curr->hook_count * COMPONENT_NAME_LEN);

				if (!hook_index) {
					memcpy(tmp, curr->hook + COMPONENT_NAME_LEN, curr->hook_count * COMPONENT_NAME_LEN);
				} else if (hook_index == (curr->hook_count + 1)) {
					memcpy(tmp, curr->hook, curr->hook_count * COMPONENT_NAME_LEN);
				} else {
					memcpy(tmp, curr->hook, hook_index * COMPONENT_NAME_LEN);
					memcpy(tmp + hook_index * COMPONENT_NAME_LEN
							, curr->hook + (hook_index + 1) * COMPONENT_NAME_LEN
							, (curr->hook_count - hook_index) * COMPONENT_NAME_LEN);
				}
				free(curr->hook);
				curr->hook = tmp;

				curr->lock = 0;
				return SANJI_SUCCESS;
			} else {
				return SANJI_LOCKED;
			}
		}
	}

	return SANJI_SUCCESS;
}

struct component *component_lookup_node_by_name(struct component *head, char *name)
{
	struct component *curr = NULL;

	if (head) {
		list_for_each_entry(curr, &head->list, list) {
			if (!strcmp(curr->name, name)) return curr;
		}
	}

	return NULL;
}

char *component_node_get_hook(struct component *node)
{
	return node->hook;
}

int component_node_get_hook_count(struct component *node)
{
	return node->hook_count;
}

char *component_get_names_by_hook(struct component *head, char *hook, unsigned int *names_count)
{
	struct component *curr = NULL;
	char *names = NULL;
	char *names_tmp = NULL;
	int i;

	*names_count = 0;

	if (head && hook) {
		list_for_each_entry(curr, &head->list, list) {
			for (i = 0; i < curr->hook_count; i++) {
				if (!strncmp(curr->hook + i * COMPONENT_NAME_LEN, hook, COMPONENT_NAME_LEN)) {
					(*names_count)++;
					names_tmp = (char *)realloc(names, (*names_count) * COMPONENT_NAME_LEN);
					if (!names_tmp) {
						DEBUG_PRINT("Error: out of memory");
						if (names) free(names);
						*names_count = 0;
						return NULL;
					}
					names = names_tmp;
					memset(names + ((*names_count) - 1) * COMPONENT_NAME_LEN, '\0', COMPONENT_NAME_LEN);
					memcpy(names + ((*names_count) - 1) * COMPONENT_NAME_LEN, curr->name + i * COMPONENT_NAME_LEN, COMPONENT_NAME_LEN);
					break;
				}
			}
		}
	}

	return names;
}

int component_node_lock(struct component *node)
{
	if (!node) {
		fprintf(stderr, "Error: component is empty.\n");
		return SANJI_DATA_ERROR;
	}

	if (node->lock) {
		fprintf(stderr, "Error: component is busy.\n");
		return SANJI_LOCKED;
	}

	node->lock = 1;
	return 0;
}

int component_node_unlock(struct component *node)
{
	if (!node) {
		fprintf(stderr, "Error: component is empty.\n");
		return SANJI_DATA_ERROR;
	}

	node->lock = 0;
	return 0;
}

int component_lock_by_name(struct component *head, char *name)
{
	struct component *curr = NULL;

	if (!head || !name) {
		fprintf(stderr, "Error: component head or name is empty.\n");
		return SANJI_DATA_ERROR;
	}

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) return component_node_lock(curr);
	}

	fprintf(stderr, "Error: component name(%s) is not registered.\n", name);
	return SANJI_NOT_FOUND;
}

int component_unlock_by_name(struct component *head, char *name)
{
	struct component *curr = NULL;

	if (!head || !name) {
		fprintf(stderr, "Error: component head or name is empty.\n");
		return SANJI_DATA_ERROR;
	}

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) return component_node_unlock(curr);
	}

	fprintf(stderr, "Error: component name(%s) is not registered.\n", name);
	return SANJI_NOT_FOUND;
}

char *component_get_tunnel_by_name(struct component *head, char *name)
{
	struct component *curr = NULL;

	if (!head || !name) {
		fprintf(stderr, "Error: component head or name is empty.\n");
		return NULL;
	}

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) return curr->tunnel;
	}

	return NULL;

}
