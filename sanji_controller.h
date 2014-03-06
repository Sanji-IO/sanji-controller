#ifndef _SANJI_CONTROLLER_H_
#define _SANJI_CONTROLLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ##########################
 * DEFINE MACRO
 * ##########################
 */
#define SANJI_VERSION "1.0.0"
#define SANJI_ERR_BUFSIZE 256
#define SANJI_HOSTNAME_BUFSIZE 256
#define SANJI_IP_LEN 16
#define SANJI_ID_LEN (MOSQ_MQTT_ID_MAX_LENGTH + 32)

/* mode of random number generator */
#define SANJI_RAND_MODE_SEQ 0
#define SANJI_RAND_MODE_RANDOM 1

/* controller topics */
#define SANJI_CONTROLLER_TOPIC "/controller"
#define SANJI_REGISTER_TOPIC "/controller/registration"
#define SANJI_REGISTER_TOPIC_LEN 24

/* sanji configurations */
#define SANJI_REFRESH_INTERVAL 1000
#define SANJI_DEFAULT_PORT 1883
#define SANJI_DEFAULT_IP "127.0.0.1"
#define SANJI_DEFAULT_KEEPALIVE 600
#define SANJI_MAX_CONTEXT_LEN (1024 * 1024)


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
	char **topics;
	int topic_count;
	int topic_qos;
	char *username;
	char *password;
	int verbose;
	bool quiet;
	bool no_retain;
	int qos_sent;
	int retain_sent;
	int mid_sent;
};


#ifdef __cplusplus
}
#endif

#endif /* _SANJI_CONTROLLER_H_ */
