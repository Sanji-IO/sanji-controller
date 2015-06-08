#ifndef _SANJI_CONTROLLER_H
#define _SANJI_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ##########################
 * DEFINE MACRO
 * ##########################
 */
#define SANJI_VERSION "1.6.0"
#define SANJI_HOSTNAME_BUFSIZE 256
#define SANJI_IP_LEN 16
#define SANJI_ID_LEN (MOSQ_MQTT_ID_MAX_LENGTH + 1)
#define SANJI_ID_SUFFIX "controller"
#define SANJI_ID_SUFFIX_LEN 11
#define SANJI_MESSAGE_LEN 256
#define SANJI_MAX_CONTEXT_LEN (1024 * 1024)
#define SANJI_MAX_PATH MAX_PATH

/* resource match algorithm */
#define SANJI_MATCH_ALG_EXACT 1
#define SANJI_MATCH_ALG_LONGEST 2

/* sanji message header */
#define SANJI_HEADER_NO_ID 0
#define SANJI_HEADER_NO_METHOD (-2)
#define SANJI_HEADER_NO_CODE (-1)

/* controller topics */
#define SANJI_CONTROLLER_NAME "controller"
#define SANJI_CONTROLLER_TOPIC "/controller"
#define SANJI_CONTROLLER_TOPIC_LEN 11
#define SANJI_TOPIC_MAX_LEN 256
#define SANJI_LOCAL_ID_LEN MOSQ_MQTT_ID_MAX_LENGTH - SANJI_ID_SUFFIX_LEN


/* build-in model topics and resources */
#define SANJI_REGISTER_NAME "registration"
#define SANJI_REGISTER_TOPIC "/controller/registration"
#define SANJI_REGISTER_TOPIC_LEN 24
#define SANJI_REGISTER_RESOURCE SANJI_REGISTER_TOPIC
#define SANJI_REGISTER_WILDCARD_RESOURCE "/controller/registration/+"
#define SANJI_REGISTER_RESOURCE_LEN 24
#define SANJI_DEPENDENCY_NAME "dependency"
#define SANJI_RESOURCE_DEPENDENCY_TOPIC "/controller/resource/dependency"
#define SANJI_RESOURCE_DEPENDENCY_TOPIC_LEN 31
#define SANJI_RESOURCE_DEPENDENCY_RESOURCE SANJI_RESOURCE_DEPENDENCY_TOPIC
#define SANJI_RESOURCE_DEPENDENCY_WILDCARD_RESOURCE "/controller/resource/dependency/+"
#define SANJI_RESOURCE_DEPENDENCY_RESOURCE_LEN 31

/* sanji configs */
#define SANJI_INI_SECTION_GLOBAL "global"
#define SANJI_INI_KEY_HOST "host"
#define SANJI_INI_KEY_PORT "port"
#define SANJI_INI_KEY_RETRY "retry"
#define SANJI_INI_KEY_KEEPALIVE "keepalive"
#define SANJI_INI_KEY_CLEAN_SESSION "clean_session"
#define SANJI_INI_KEY_SUB_QOS "sub_qos"
#define SANJI_INI_KEY_PUB_QOS "pub_qos"
#define SANJI_INI_KEY_CLIENT_ID "client_id"
#define SANJI_INI_KEY_USERNAME "username"
#define SANJI_INI_KEY_PASSWORD "password"
#define SANJI_INI_KEY_REFRESH_INTERVAL "refresh_interval"
#define SANJI_INI_KEY_LOCAL_ID "local_id"
#define SANJI_INI_KEY_MOSQ_DEBUG "mosq_debug"
#define SANJI_INI_VALUE_LEN 128
#define SANJI_DEFAULT_HOST "127.0.0.1"
#define SANJI_DEFAULT_PORT 1883
#define SANJI_DEFAULT_KEEPALIVE 600
#define SANJI_DEFAULT_RETRY (-1)
#define SANJI_DEFAULT_SUB_QOS 2
#define SANJI_DEFAULT_PUB_QOS 1
#define SANJI_DEFAULT_REFRESH_INTERVAL 1000
#define SANJI_DEFAULT_CONFIG_FILE "/etc/sanji_controller.conf"


/*
 * ##########################
 * DECLARE STRUCTURE
 * ##########################
 */

/*
 * This struct is used to pass data to callbacks.
 * An instance "ud" is created in main() and populated, then passed to
 * mosquitto_new().
 */
struct sanji_userdata {
	char client_id[SANJI_ID_LEN];
	char local_id_topic[SANJI_TOPIC_MAX_LEN];
	char **topics;
	int topic_count;
	int sub_qos;
	int pub_qos;
	int *topic_mids;
	int retain_sent;
	int mid_sent;
};

/*
 * This struct is used to store all configuration options
 */
struct sanji_config {
	/* connect */
	char host[SANJI_IP_LEN];
	int port;
	int retry;
	/* mosquitto */
	int keepalive;
	bool clean_session;
	int sub_qos;
	int pub_qos;
	char *client_id;
	char *username;
	char *password;
	/* sanji controller */
	int refresh_interval;
	/* misc */
	char *local_id;
	char *config_file;
	bool foreground;
	bool mosq_debug;
};


#ifdef __cplusplus
}
#endif

#endif /* _SANJI_CONTROLLER_H */
