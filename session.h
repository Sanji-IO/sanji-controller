#ifndef _SANJI_SESSION_H_
#define _SANJI_SESSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <jansson.h>
#include "sanji_controller.h"
#include "list.h"
#include "debug.h"
#include "http.h"
#include "resource.h"
#include "component.h"

/* session return code */
#define SESSION_CODE_OK 					HTTP_OK
#define SESSION_CODE_BAD_REQUEST			HTTP_BAD_REQUEST
#define SESSION_CODE_FORBIDDEN				HTTP_FORBIDDEN
#define SESSION_CODE_NOT_FOUND				HTTP_NOT_FOUND
#define SESSION_CODE_METHOD_NOT_ALLOWED		HTTP_METHOD_NOT_ALLOWED
#define SESSION_CODE_REQUEST_TIMEOUT		HTTP_REQUEST_TIME_OUT
#define SESSION_CODE_LOCKED					HTTP_LOCKED
#define SESSION_CODE_INTERNAL_SERVER_ERROR	HTTP_INTERNAL_SERVER_ERROR
#define SESSION_CODE_NOT_IMPLEMENTED		HTTP_NOT_IMPLEMENTED
#define SESSION_CODE_SERVICE_UNAVAILABLE	HTTP_SERVICE_UNAVAILABLE

#define SESSION_MAX_ID 4294967295
#define SESSION_MIN_ID 1

struct model_chain {
	char *models;
	unsigned int count;
	int *ttls;
};

struct session {
	struct list_head list;
	unsigned int id;
	int method;
	char resource[RESOURCE_NAME_LEN];
	char tunnel[COMPONENT_TUNNEL_LEN];
	char *dependency_chain;
	unsigned int dependency_chain_count;
	json_t *result_chain;	// include 'code' and 'data'
	struct model_chain *model_chain;
	unsigned int model_chain_count;
	char *view_chain;
	unsigned int view_chain_count;
	unsigned int curr_step;
	unsigned int curr_wait;
};

struct session *session_init();
struct session *session_create_node(unsigned int, int, char *, char *, char *, unsigned int, json_t *, struct model_chain *, unsigned int, char *, unsigned int);
int session_add(struct session *, struct session *);
int session_add_node(struct session *, unsigned int, int, char *, char *, char *, unsigned int, json_t *, struct model_chain *, unsigned int, char *, unsigned int);
struct session *session_lookup_node_by_id(struct session *, unsigned int);
void session_display(struct session *);
void session_display_model_chain(struct model_chain *, unsigned int);
void session_display_view_chain(char *, unsigned int);
int session_delete_first_resource(struct session *, char *);
int session_delete_first_id(struct session *, unsigned int);

/* inspect method */
int session_is_inflight(struct session *, unsigned int);

/* lock method */
int session_node_lock_by_step(struct session *, struct resource *, struct component *, unsigned int);
int session_node_unlock_by_step(struct session *, struct resource *, struct component *, unsigned int);

/* get method */

/* free method */
void session_free(struct session *);
void session_free_model_chain(struct model_chain *, unsigned int);

/* step method */
void session_node_update_wait(struct session *);
void session_node_decref_wait(struct session *, struct session *, struct resource *, struct component *, struct mosquitto *, void *, json_t *);
int session_step(struct session *, struct session *, struct resource *, struct component *, struct mosquitto *, void *, json_t *);
void session_step_update(struct session *, struct session *, struct resource *, struct component *, struct mosquitto *, void *, json_t *, int, json_t *);
int session_step_stop(struct session *,	struct session *, struct resource *, struct component *, struct mosquitto *, void *, json_t *);

/* ttl method */
void session_decref_ttl(struct session *, struct resource *, struct component *, struct mosquitto *, void *, unsigned int);


#ifdef __cplusplus
}
#endif

#endif /* _SANJI_SESSION_H_ */
