#include "resource.h"

struct resource *resource_init()
{
	struct resource *head = NULL;

	head = (struct resource *)malloc(sizeof(struct resource));
	if  (!head) {
		fprintf(stderr, "Error: out of memory.\n");
		return NULL;
	}

	/* initialize list */
	INIT_LIST_HEAD(&head->list);

	return head;
}

int resource_add_node(struct resource *head, char *name, char *subscribed_component, unsigned int subscribed_count, unsigned int lock)
{
	struct resource *node = NULL;

	if (!head) {
		fprintf(stderr, "Error: add node failed, list head is empty\n");
		return 1;
	}
	
	node = (struct resource *)malloc(sizeof(struct resource));
	if  (!node) {
		fprintf(stderr, "Error: add node failed, out of memory.\n");
		return 1;
	}
	memset(node, 0, sizeof(struct resource));

	INIT_LIST_HEAD(&node->list);
	if (name) strncpy(node->name, name, RESOURCE_NAME_LEN - 1);
	node->name[RESOURCE_NAME_LEN - 1] = '\0';
	node->lock = lock;

	node->subscribed_count = subscribed_count;
	if (subscribed_component) {
		node->subscribed_component = malloc(subscribed_count * COMPONENT_NAME_LEN);
		if  (!node->subscribed_component) {
			fprintf(stderr, "Error: add node failed, out of memory.\n");
			free(node);
			return 1;
		}
		memset(node->subscribed_component, '\0', subscribed_count * COMPONENT_NAME_LEN);
		memcpy(node->subscribed_component, subscribed_component, subscribed_count * COMPONENT_NAME_LEN);
		node->subscribed_component[subscribed_count * COMPONENT_NAME_LEN - 1] = '\0';
	}

	list_add_tail(&node->list, &head->list);

	return 0;
}

void resource_display(struct resource *head)
{
	struct resource *curr = NULL;
	int i = 1, j;

	if (head) {
		list_for_each_entry(curr, &head->list, list) {
			fprintf(stderr, "resource node(%d):\n", i++);
			fprintf(stderr, "\tname(%s)\n", curr->name);
			for (j = 0; j < curr->subscribed_count; j++) {
				fprintf(stderr, "\tsubscribed_component(%s)\n", curr->subscribed_component + (j * COMPONENT_NAME_LEN));
			}
			fprintf(stderr, "\tsubscribed_count(%d)\n", curr->subscribed_count);
			fprintf(stderr, "\tlock(%d)\n", curr->lock);
		}
	}
}

void resource_free(struct resource *head)
{
	struct resource *curr = NULL;

	if (head) {
		while (!list_empty(&head->list)) {
			curr = list_entry(head->list.next, struct resource, list);
			list_del(&curr->list);
			if (curr->subscribed_component) free(curr->subscribed_component);
			free(curr);
		}
		free(head);
	}
}

int resource_delete_first_name(struct resource *head, char *name)
{
	struct resource *curr = NULL;

	if (!head || !name) {
		fprintf(stderr, "Error: list head or name which wanted deleted is empty.\n");
		return SANJI_DATA_ERROR;
	}

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) {
			if (!curr->lock) {
				list_del(&curr->list);
				if (curr->subscribed_component) free(curr->subscribed_component);
				free(curr);
				return SANJI_SUCCESS;
			} else {
				return SANJI_LOCKED;
			}
		}
	}

	return SANJI_SUCCESS;
}

int resource_lookup_method(const char *method)
{
	/* Note: the following code was generated by the "shilka" tool from
	   the "cocom" parsing/compilation toolkit. It is an optimized lookup
	   based on analysis of the input keywords. Postprocessing was done
	   on the shilka output, but the basic structure and analysis is
	   from there. Should new SANJI methods be added, then manual insertion
	   into this code is fine, or simply re-running the shilka tool on
	   the appropriate input. */

	char method_upper[RESOURCE_METHOD_LEN];
  	unsigned int method_len;
	int i;

	if (!method) return RESOURCE_METHOD_UNKNOWN;
	method_len = strlen(method);

	/* to upper case */
	memset(method_upper, '\0', RESOURCE_METHOD_LEN);
	for (i = 0; method[i]; i++) {
		method_upper[i] = toupper(method[i]);
	}

	switch (method_len) {
	case 3:
		switch (method_upper[0]) {
		case 'P':
			return (method_upper[1] == 'U'
					&& method_upper[2] == 'T'
					? RESOURCE_METHOD_UPDATE : RESOURCE_METHOD_UNKNOWN);
		case 'G':
			return (method_upper[1] == 'E'
					&& method_upper[2] == 'T'
					? RESOURCE_METHOD_READ : RESOURCE_METHOD_UNKNOWN);
		default:
			return RESOURCE_METHOD_UNKNOWN;
		}

	case 4:
		switch (method_upper[0]) {
		case 'P':
			return (method_upper[1] == 'O'
					&& method_upper[2] == 'S'
					&& method_upper[3] == 'T'
					? RESOURCE_METHOD_CREATE : RESOURCE_METHOD_UNKNOWN);
		default:
			return RESOURCE_METHOD_UNKNOWN;
		}

	case 6:
		switch (method_upper[0]) {
		case 'D':
			return (memcmp(method_upper, "DELETE", 6) == 0
					? RESOURCE_METHOD_DELETE : RESOURCE_METHOD_UNKNOWN);
		default:
			return RESOURCE_METHOD_UNKNOWN;
		}

	default:
		return RESOURCE_METHOD_UNKNOWN;
	}
}

/* This function will allocate memory and return.
 * Must free return address */
char *resource_reverse_method(const int method)
{
	char *_method = NULL;

	_method = (char *)malloc(RESOURCE_METHOD_LEN * sizeof(char));
	if (!_method) {
		fprintf(stderr, "Error: reverse method failed, out of memory.\n");
		return NULL;
	}
	memset(_method, '\0', RESOURCE_METHOD_LEN * sizeof(char));

	switch (method) {
	case RESOURCE_METHOD_CREATE:
		sprintf(_method, RESOURCE_METHOD_TEXT_CREATE);
		return _method;
		break;
	case RESOURCE_METHOD_READ:
		sprintf(_method, RESOURCE_METHOD_TEXT_READ);
		return _method;
		break;
	case RESOURCE_METHOD_UPDATE:
		sprintf(_method, RESOURCE_METHOD_TEXT_UPDATE);
		return _method;
		break;
	case RESOURCE_METHOD_DELETE:
		sprintf(_method, RESOURCE_METHOD_TEXT_DELETE);
		return _method;
		break;
	default:
		break;
	}

	free(_method);
	return NULL;
}

int resource_is_write_like_method(int method)
{
	if (method == RESOURCE_METHOD_READ) return 0;

	return 1;
}

int resource_is_registered(struct resource *head, char *name)
{
	struct resource *curr = NULL;

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

int resource_is_locked(struct resource *head, char *name)
{
	struct resource *curr = NULL;

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) {
			return curr->lock;
		}
	}

	return 0;
}

int resource_node_is_locked(struct resource *node)
{
	if (node) return node->lock;
	return 0;
}


int resource_is_any_locked_by_component(struct resource *head, char *subscribed_component)
{
	struct resource *curr = NULL;
	int is_locked = 0;
	int is_finded = 0;
	int i;

	list_for_each_entry(curr, &head->list, list) {
		/* clear varirable */
		is_finded = 0;

		/* find the index of subscribed component */
		for (i = 0; i < curr->subscribed_count; i++) {
			if (!strncmp(curr->subscribed_component + i * COMPONENT_NAME_LEN
						, subscribed_component
						, COMPONENT_NAME_LEN)) {
				is_finded = 1;
				break;
			}
		}
		if (is_finded) is_locked += curr->lock;
		if (is_locked) break;
	}

	return is_locked;
}

int resource_append_component_by_name(struct resource *head, char *name, char *subscribed_component)
{
	struct resource *curr = NULL;
	int is_registered;
	char *tmp = NULL;

	if (!head || !name || !subscribed_component) {
		fprintf(stderr, "Error: append subscribed component failed\n");
		return SANJI_DATA_ERROR;
	}

	/* check whether already been registered */
	is_registered = resource_is_registered(head, name);
	if (is_registered == -1) {
		return SANJI_DATA_ERROR;
	} else if (!is_registered) {
		return resource_add_node(head, name, subscribed_component, 1, 0);
	}
	
	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) {
			if (!curr->lock) {
				curr->lock = 1;
				/*
				 * Reallocate memory.
				 * Note, be careful to use realloc().
				 */
				curr->subscribed_count++;
				tmp = realloc(curr->subscribed_component, curr->subscribed_count * COMPONENT_NAME_LEN);
				if (!tmp) {
					fprintf(stderr, "Error: append node failed, out of memory.\n");
					return SANJI_DATA_ERROR;
				}
				curr->subscribed_component = tmp;
				memset(curr->subscribed_component + (curr->subscribed_count - 1) * COMPONENT_NAME_LEN, '\0', COMPONENT_NAME_LEN);
				memcpy(curr->subscribed_component + (curr->subscribed_count - 1) * COMPONENT_NAME_LEN, subscribed_component, COMPONENT_NAME_LEN);
				curr->subscribed_component[curr->subscribed_count * COMPONENT_NAME_LEN - 1] = '\0';
				curr->lock = 0;
				return SANJI_SUCCESS;
			} else {
				return SANJI_LOCKED;
			}
		}
	}

	return SANJI_SUCCESS;
}

int resource_remove_component_by_name(struct resource *head, char *name, char *subscribed_component)
{
	struct resource *curr = NULL;
	int is_registered;
	int is_finded = 0;
	int component_index = -1;
	char *tmp = NULL;
	int i;

	if (!head || !name || !subscribed_component) {
		fprintf(stderr, "Error: remove subscribed component failed\n");
		return SANJI_DATA_ERROR;
	}

	/* check whether already been registered */
	is_registered = resource_is_registered(head, name);
	if (is_registered == -1) {
		return SANJI_DATA_ERROR;
	} else if (!is_registered) {
		return SANJI_SUCCESS;;
	}
	
	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) {
			if (!curr->lock) {
				curr->lock = 1;

				/* check if subscribed_count is already '0' or '1', just free it. */
				if (!curr->subscribed_count || (curr->subscribed_count == 1)) {
					list_del(&curr->list);
					if (curr->subscribed_component)
						free(curr->subscribed_component);
					free(curr);
					return SANJI_SUCCESS;
				}

				/* find the index of subscribed component */
				for (i = 0; i < curr->subscribed_count; i++) {
					if (!strncmp(curr->subscribed_component + i * COMPONENT_NAME_LEN
								, subscribed_component
								, COMPONENT_NAME_LEN)) {
						is_finded = 1;
						component_index = i;
						break;
					}
				}

				if (!is_finded) {
					curr->lock = 0;
					return SANJI_SUCCESS;
				}
				curr->subscribed_count--;

				/* cascade subscribed component */
				tmp = malloc(curr->subscribed_count * COMPONENT_NAME_LEN);
				if (!tmp) {
					fprintf(stderr, "Error: remove node failed, out of memory.\n");
					curr->lock = 0;
					return SANJI_DATA_ERROR;
				}
				memset(tmp, '\0', curr->subscribed_count * COMPONENT_NAME_LEN);

				if (!component_index) {
					memcpy(tmp, curr->subscribed_component + COMPONENT_NAME_LEN, curr->subscribed_count * COMPONENT_NAME_LEN);
				} else if (component_index == (curr->subscribed_count + 1)) {
					memcpy(tmp, curr->subscribed_component, curr->subscribed_count * COMPONENT_NAME_LEN);
				} else {
					memcpy(tmp, curr->subscribed_component, component_index * COMPONENT_NAME_LEN);
					memcpy(tmp + component_index * COMPONENT_NAME_LEN
							, curr->subscribed_component + (component_index + 1) * COMPONENT_NAME_LEN
							, (curr->subscribed_count - component_index) * COMPONENT_NAME_LEN);
				}
				free(curr->subscribed_component);
				curr->subscribed_component = tmp;

				curr->lock = 0;
				return SANJI_SUCCESS;
			} else {
				return SANJI_LOCKED;
			}
		}
	}

	return SANJI_SUCCESS;
}

int resource_remove_all_component_by_name(struct resource *head, char *subscribed_component)
{
	struct resource *curr = NULL;
	struct resource **remove_resource = NULL;
	int remove_count = 0;
	int *remove_component_index = NULL;
	char *tmp = NULL;
	int i;

	if (!head || !subscribed_component) {
		fprintf(stderr, "Error: remove subscribed component failed\n");
		return SANJI_DATA_ERROR;
	}

	list_for_each_entry(curr, &head->list, list) {
		/* find the index of subscribed component */
		for (i = 0; i < curr->subscribed_count; i++) {
			if (!strncmp(curr->subscribed_component + i * COMPONENT_NAME_LEN
						, subscribed_component
						, COMPONENT_NAME_LEN)) {
				remove_count++;
				remove_resource = realloc(remove_resource, remove_count * sizeof(struct resource *));
				remove_resource[remove_count - 1] = curr;
				remove_component_index = realloc(remove_component_index, remove_count * sizeof(int));
				remove_component_index[remove_count - 1] = i;
				break;
			}
		}
	}

	/* remove component from all resources */
	for (i = 0; i < remove_count; i++) {

		/* reuse 'curr' resource pointer */
		curr = remove_resource[i];

		curr->subscribed_count--;

		if (!curr->lock) {
			curr->lock = 1;

			/* check if subscribed_count is already '0', just free it. */
			if (!curr->subscribed_count) {
				list_del(&curr->list);
				if (curr->subscribed_component)
					free(curr->subscribed_component);
				free(curr);
				continue;
			}

			/* cascade subscribed component */
			tmp = malloc(curr->subscribed_count * COMPONENT_NAME_LEN);
			if (!tmp) {
				fprintf(stderr, "Error: remove node failed, out of memory.\n");
				curr->lock = 0;
				if (remove_resource) free(remove_resource);
				if (remove_component_index) free(remove_component_index);
				return SANJI_DATA_ERROR;
			}
			memset(tmp, '\0', curr->subscribed_count * COMPONENT_NAME_LEN);

			if (!remove_component_index[i]) {
				memcpy(tmp, curr->subscribed_component + COMPONENT_NAME_LEN, curr->subscribed_count * COMPONENT_NAME_LEN);
			} else if (remove_component_index[i] == (curr->subscribed_count + 1)) {
				memcpy(tmp, curr->subscribed_component, curr->subscribed_count * COMPONENT_NAME_LEN);
			} else {
				memcpy(tmp, curr->subscribed_component, remove_component_index[i] * COMPONENT_NAME_LEN);
				memcpy(tmp + remove_component_index[i] * COMPONENT_NAME_LEN
						, curr->subscribed_component + (remove_component_index[i] + 1) * COMPONENT_NAME_LEN
						, (curr->subscribed_count - remove_component_index[i]) * COMPONENT_NAME_LEN);
			}
			free(curr->subscribed_component);
			curr->subscribed_component = tmp;

			curr->lock = 0;
		} else {
			if (remove_resource) free(remove_resource);
			if (remove_component_index) free(remove_component_index);
			return SANJI_LOCKED;
		}
	}

	/* free */
	if (remove_resource) free(remove_resource);
	if (remove_component_index) free(remove_component_index);

	return SANJI_SUCCESS;
}

struct resource *resource_lookup_node_by_name(struct resource *head, const char *name)
{
	struct resource *curr = NULL;

	if (head) {
		list_for_each_entry(curr, &head->list, list) {
			if (!strcmp(curr->name, name)) return curr;
		}
	}

	return NULL;
}

/* lookup for wildcard '#' and '+' */
struct resource **resource_lookup_wildcard_nodes_by_name(struct resource *head, const char *name, unsigned int *resources_count)
{
	struct resource *curr = NULL;
	struct resource **resources = NULL;
	char resource_name[RESOURCE_NAME_LEN];
	char _name[RESOURCE_NAME_LEN];
	char *p, *q;

	*resources_count = 0;

	if (head) {
		list_for_each_entry(curr, &head->list, list) {
			/* deep copy curr->name */
			memset(resource_name, '\0', RESOURCE_NAME_LEN);
			strncpy(resource_name, curr->name, RESOURCE_NAME_LEN);
			memset(_name, '\0', RESOURCE_NAME_LEN);
			strncpy(_name, name, RESOURCE_NAME_LEN);

			/* truncate '?' */
			p = strrchr(_name, '?');
			if (p) *p = '\0';

			p = resource_name;
			q = _name;
			/* start lookup */
			while (*p && *q) {
				if (*p == '#') {
					break;
				} else if (*p == '+') {
					if (*q == '/') break;

					p++;
					while ((*q != '/') && *q) {
						q++;
					}
				} else {
					if (*p != *q) break;
					p++;
					q++;
				}
			}

			if ((*p == '#' && *q) || (!*p && !*q)) {
				(*resources_count)++;
				resources = realloc(resources, (*resources_count) * sizeof(struct resource *));
				resources[(*resources_count) - 1] = curr;
			}
		}
	}

	return resources;
}

char *resource_get_subscribed_component(struct resource *node)
{
	if (!node) return NULL;
	return node->subscribed_component;
}

unsigned int resource_get_subscribed_count(struct resource *node)
{
	if (!node) return 0;
	return node->subscribed_count;
}

int resource_node_lock(struct resource *node)
{
	if (!node) {
		fprintf(stderr, "Error: resource is empty.\n");
		return SANJI_DATA_ERROR;
	}

	if (node->lock) {
		fprintf(stderr, "Error: resource is busy.\n");
		return SANJI_LOCKED;
	}

	node->lock = 1;
	return 0;
}

int resource_node_unlock(struct resource *node)
{
	if (!node) {
		fprintf(stderr, "Error: resource is empty.\n");
		return SANJI_DATA_ERROR;
	}

	node->lock = 0;
	return 0;
}

int resource_lock_by_name(struct resource *head, char *name)
{
	struct resource *curr = NULL;

	if (!head || !name) {
		fprintf(stderr, "Error: resource head or name is empty.\n");
		return SANJI_DATA_ERROR;
	}

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) return resource_node_lock(curr);
	}

	fprintf(stderr, "Error: resource name(%s) is not registered.\n", name);
	return SANJI_NOT_FOUND;
}

int resource_unlock_by_name(struct resource *head, char *name)
{
	struct resource *curr = NULL;

	if (!head || !name) {
		fprintf(stderr, "Error: resource head or name is empty.\n");
		return SANJI_DATA_ERROR;
	}

	list_for_each_entry(curr, &head->list, list) {
		if (!strcmp(curr->name, name)) return resource_node_unlock(curr);
	}

	fprintf(stderr, "Error: resource name(%s) is not registered.\n", name);
	return SANJI_NOT_FOUND;
}

char *resource_get_names_by_component(struct resource *head, char *component, unsigned int *names_count)
{
	struct resource *curr = NULL;
	char *names = NULL;
	char *names_tmp = NULL;
	int i;

	*names_count = 0;

	if (head && component) {
		list_for_each_entry(curr, &head->list, list) {
			for (i = 0; i < curr->subscribed_count; i++) {
				if (!strncmp(curr->subscribed_component + i * COMPONENT_NAME_LEN, component, COMPONENT_NAME_LEN)) {
					(*names_count)++;
					names_tmp = (char *)realloc(names, (*names_count) * RESOURCE_NAME_LEN);
					if (!names_tmp) {
						DEBUG_PRINT("Error: out of memory");
						if (names) free(names);
						*names_count = 0;
						return NULL;
					}
					names = names_tmp;
					memset(names + ((*names_count) - 1) * RESOURCE_NAME_LEN, '\0', RESOURCE_NAME_LEN);
					memcpy(names + ((*names_count) - 1) * RESOURCE_NAME_LEN, curr->name, RESOURCE_NAME_LEN);
					break;
				}
			}
		}
	}

	return names;
}

