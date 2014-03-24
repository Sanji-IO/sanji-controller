#ifndef _SANJI_COMPONENT_H_
#define _SANJI_COMPONENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "debug.h"
#include "error.h"

#define COMPONENT_NAME_LEN 32
#define COMPONENT_DESCRIPTION_LEN 256
#define COMPONENT_TUNNEL_LEN 64
#define COMPONENT_ROLE_LEN 16

struct component {
	struct list_head list;
	char name[COMPONENT_NAME_LEN];
	char description[COMPONENT_DESCRIPTION_LEN];
	char tunnel[COMPONENT_TUNNEL_LEN];
	char role[COMPONENT_ROLE_LEN];
	char *hook;
	unsigned int hook_count;
	int ttl;
	unsigned int lock;
};

struct component *component_init();
int component_add_node(struct component *, char *, char *, char *, char *, char *, unsigned int, int, unsigned int);
void component_display(struct component *);
void component_free(struct component *);
int component_delete_first_name(struct component *, char *);
int component_append_hook_by_name(struct component *, char *, char *);
int component_remove_hook_by_name(struct component *, char *, char *);
struct component *component_lookup_node_by_name(struct component *, char *);

/* lock method */
int component_node_lock(struct component *);
int component_lock_by_name(struct component *, char *);
int component_node_unlock(struct component *);
int component_unlock_by_name(struct component *, char *);

/* inspect method */
int component_is_registered(struct component *, char *);
int component_is_locked(struct component *, char *);
int component_node_is_locked(struct component *);
int component_is_given_role(struct component *, char *, char *);
int component_node_is_given_role(struct component *, char *);
int component_is_unique_tunnel(struct component *, char *);

/* get method */
char *component_node_get_hook(struct component *);
int component_node_get_hook_count(struct component *);
char *component_get_names_by_hook(struct component *, char *, unsigned int *);
char *component_get_tunnel_by_name(struct component *, char *);



#ifdef __cplusplus
}
#endif

#endif /* _SANJI_COMPONENT_H_ */
