#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mosquitto.h>

#ifndef WIN32
#  include <unistd.h>
#else
#  include <process.h>
#  define snprintf sprintf_s
#endif

#define mqtt_host "127.0.0.1"
#define mqtt_port 1883
#define mqtt_clinetid "client_id"
//#define mqtt_topic "#"
#define mqtt_topic "simon"

static int run = 1;

void mqtt_signal_handler(int s)
{
	fprintf(stderr, "MQTT: signal_handler: get stop signal.\n");
	run = 0;
}

void mqtt_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	if (!result) {
		fprintf(stderr, "MQTT: connect_callback: success.\n");
	} else {
		fprintf(stderr, "MQTT: connect_callback: failed.\n");
	}
}

void mqtt_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	fprintf(stderr, "MQTT: subscribe_callback: subscribed mid(%d)\n", mid);
}


void mqtt_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	fprintf(stderr, "MQTT: message_callback: Get a message, mid(%d), topic(%s), message(", message->mid, message->topic);
	fwrite(message->payload, 1, message->payloadlen, stderr);
	fprintf(stderr, ")\n");
}

int main(int argc, char *argv[])
{
	char clientid[24];
	int messageid;
	struct mosquitto *mosq = NULL;
	int rc = 0;

	/* setup signal handler */
	signal(SIGINT, mqtt_signal_handler);
	signal(SIGTERM, mqtt_signal_handler);

	/* set client id */
	memset(clientid, '\0', sizeof(clientid));
	snprintf(clientid, 23, "%s_%d", mqtt_clinetid, getpid());

	/* init mosquitto library */
	mosquitto_lib_init();

	/* create mosquitto */
	mosq = mosquitto_new(clientid, true, NULL);
	if (!mosq) {
		fprintf(stderr, "Error: CANNOT create mosquitto\n");
		mosquitto_lib_cleanup();
		exit(1);
	}
	fprintf(stderr, "MQTT: create a mosquitto instance with client id('%s')\n", clientid);

	/* setup call back function */
	mosquitto_connect_callback_set(mosq, mqtt_connect_callback);
	mosquitto_subscribe_callback_set(mosq, mqtt_subscribe_callback);
	mosquitto_message_callback_set(mosq, mqtt_message_callback);

	/* connet mosquitto */
	rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);
	if (rc != MOSQ_ERR_SUCCESS) {
		fprintf(stderr, "Error: CANNOT connet to  mosquitto broker\n");
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();
		exit(1);
	}
	fprintf(stderr, "MQTT: connect to server client ip(%s:%d)\n", mqtt_host, mqtt_port);
	
	/* subcribe topic */
	fprintf(stderr, "MQTT: subscribe topic('%s')\n", mqtt_topic);
	mosquitto_subscribe(mosq, &messageid, mqtt_topic, 0);
	fprintf(stderr, "MQTT: message id('%d')\n", messageid);

	while (run) {
		rc = mosquitto_loop(mosq, -1, 1);
		if (run && rc) {
			fprintf(stderr, "MQTT: reconnect to server\n");
			mosquitto_reconnect(mosq);
		}
	}

	fprintf(stderr, "MQTT: stopping program\n");

	/* destroy mosquitto */
	mosquitto_destroy(mosq);

	/* clean up mosquitto library */
	mosquitto_lib_cleanup();

	return 0;
}

