#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#include <winsock2.h>
#define snprintf sprintf_s
#endif

#include <mosquitto.h>

#define MQTT_VERSION "1.0.0"
#define MQTT_ERR_BUFSIZE 256
#define MQTT_HOSTNAME_BUFSIZE 256
#define MQTT_IP_LEN 16
#define MQTT_ID_LEN (MOSQ_MQTT_ID_MAX_LENGTH + 32)

static int run = 1;

/*
 * This struct is used to pass data to callbacks.
 * An instance "ud" is created in main() and populated, then passed to
 * mosquitto_new().
 */
struct mqtt_userdata {
	char **topics;
	int topic_count;
	int topic_qos;
	char *username;
	char *password;
	int verbose;
	bool quiet;
	bool no_retain;
};

/*
 * ##########################
 * Callback Functions
 * ##########################
 */
void mqtt_signal_handler(int s)
{
	fprintf(stderr, "MQTT: signal_handler: get stop signal.\n");
	run = 0;
}

void mqtt_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	struct mqtt_userdata *ud = NULL;

	assert(obj);
	ud = (struct mqtt_userdata *)obj;

	if(message->retain && ud->no_retain) return;

	if(ud->verbose){
		if(message->payloadlen){
			fprintf(stderr, "============================================================\n", message->topic);
			fprintf(stderr, "[%s]\n\n", message->topic);
			fprintf(stderr, "%s\n\n", message->payload);
		}else{
			fprintf(stderr, "============================================================\n", message->topic);
			fprintf(stderr, "[%s]\n\n", message->topic);
			fprintf(stderr, "(null)\n\n");
		}
	}else{
		if(message->payloadlen){
			fwrite(message->payload, 1, message->payloadlen, stdout);
			printf("\n");
			fflush(stdout);
		}
	}
}

void mqtt_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	int i;
	struct mqtt_userdata *ud = NULL;

	assert(obj);
	ud = (struct mqtt_userdata *)obj;

	if(!result){
		for(i=0; i<ud->topic_count; i++){
			mosquitto_subscribe(mosq, NULL, ud->topics[i], ud->topic_qos);
		}
	}else{
		if(result && !ud->quiet){
			fprintf(stderr, "%s\n", mosquitto_connack_string(result));
		}
	}
}

void mqtt_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	int i;
	struct mqtt_userdata *ud = NULL;

	assert(obj);
	ud = (struct mqtt_userdata *)obj;

	if(!ud->quiet) printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		if(!ud->quiet) printf(", %d", granted_qos[i]);
	}
	if(!ud->quiet) printf("\n");
}

void mqtt_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	printf("MQTT: log_callback: %s\n", str);
}

/*
 * ##########################
 * Functions
 * ##########################
 */
void mqtt_userdata_free(struct mqtt_userdata *ud)
{
	if (ud) {
		if (ud->topics) {
			free(ud->topics);
		}
		free(ud);
	}
}

void mqtt_print_usage(void)
{
	int major, minor, revision;

	mosquitto_lib_version(&major, &minor, &revision);
	printf("mosquitto_sub is a simple mqtt client that will subscribe to a single topic and print all messages it receives.\n");
	printf("mosquitto_sub version %s running on libmosquitto %d.%d.%d.\n\n", MQTT_VERSION, major, minor, revision);
	printf("Usage: mosquitto_sub [-c] [-h host] [-k keepalive] [-p port] [-q qos] [-R] [-v] -t topic ...\n");
	printf("                     [-i id] [-I id_prefix]\n");
	printf("                     [-d] [--quiet]\n");
	printf("                     [-u username [-P password]]\n");
	printf("                     [--will-topic [--will-payload payload] [--will-qos qos] [--will-retain]]\n");
	printf("       mosquitto_sub --help\n\n");
	printf(" -c : disable 'clean session' (store subscription and pending messages when client disconnects).\n");
	printf(" -d : enable debug messages.\n");
	printf(" -h : mqtt host to connect to. Defaults to localhost.\n");
	printf(" -i : id to use for this client. Defaults to mosquitto_sub_ appended with the process id.\n");
	printf(" -I : define the client id as id_prefix appended with the process id. Useful for when the\n");
	printf("      broker is using the clientid_prefixes option.\n");
	printf(" -k : keep alive in seconds for this client. Defaults to 60.\n");
	printf(" -p : network port to connect to. Defaults to 1883.\n");
	printf(" -q : quality of service level to use for the subscription. Defaults to 0.\n");
	printf(" -R : do not print stale messages (those with retain set).\n");
	printf(" -t : mqtt topic to subscribe to. May be repeated multiple times.\n");
	printf(" -u : provide a username (requires MQTT 3.1 broker)\n");
	printf(" -v : print published messages verbosely.\n");
	printf(" -P : provide a password (requires MQTT 3.1 broker)\n");
	printf(" --help : display this message.\n");
	printf(" --quiet : don't print error messages.\n");
	printf(" --will-payload : payload for the client Will, which is sent by the broker in case of\n");
	printf("                  unexpected disconnection. If not given and will-topic is set, a zero\n");
	printf("                  length message will be sent.\n");
	printf(" --will-qos : QoS level for the client Will.\n");
	printf(" --will-retain : if given, make the client Will retained.\n");
	printf(" --will-topic : the topic on which to publish the client Will.\n");
	printf("\nSee http://mosquitto.org/ for more information.\n\n");
}

/*
 * ##########################
 * Main Function
 * ##########################
 */
int main(int argc, char *argv[])
{
	/* program setup variable */
	struct mosquitto *mosq = NULL;
	struct mqtt_userdata *ud = NULL;
	bool clean_session = true;
	bool debug = false;
	char err[MQTT_ERR_BUFSIZE];
	/* client id */
	char id[MQTT_ID_LEN];
	char id_prefix[MQTT_ID_LEN];
	char hostname[MQTT_HOSTNAME_BUFSIZE];
	/* broker variable */
	char host[MQTT_IP_LEN] = "127.0.0.1";
	int port = 1883;
	int keepalive = 3600;
	/* will information */
	char *will_topic = NULL;
	long will_payloadlen = 0;
	char *will_payload = NULL;
	int will_qos = 0;
	bool will_retain = false;
	/* temp variable */
	int i;
	int rc;

	/* initialized program and user data structure */
	ud = malloc(sizeof(struct mqtt_userdata));
	memset(ud, 0, sizeof(struct mqtt_userdata));
	memset(id, '\0', sizeof(id));
	memset(id_prefix, '\0', sizeof(id_prefix));

	/* get option */
	for(i=1; i<argc; i++){
		if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port")){
			if(i==argc-1){
				fprintf(stderr, "Error: -p argument given but no port specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				port = atoi(argv[i+1]);
				if(port<1 || port>65535){
					fprintf(stderr, "Error: Invalid port given: %d\n", port);
					mqtt_print_usage();
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "--disable-clean-session")){
			clean_session = false;
		}else if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")){
			debug = true;
		}else if(!strcmp(argv[i], "--help")){
			mqtt_print_usage();
			return 0;
		}else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--host")){
			if(i==argc-1){
				fprintf(stderr, "Error: -h argument given but no host specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				if (strlen(argv[i+1]) >= MQTT_IP_LEN) {
					fprintf(stderr, "Error: max length of ip is %d.\n\n", MQTT_IP_LEN);
					mqtt_print_usage();
				} else {
					memset(host, '\0', sizeof(host));
					strcpy(host, argv[i+1]);
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--id")){
			if(strlen(id_prefix) != 0){
				fprintf(stderr, "Error: -i and -I argument cannot be used together.\n\n");
				mqtt_print_usage();
				return 1;
			}
			if(i==argc-1){
				fprintf(stderr, "Error: -i argument given but no id specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				if (strlen(argv[i+1]) >= MOSQ_MQTT_ID_MAX_LENGTH) {
					fprintf(stderr, "Error: max length of client id is %d.\n\n", MOSQ_MQTT_ID_MAX_LENGTH);
					mqtt_print_usage();
				} else {
					strcpy(id, argv[i+1]);
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-I") || !strcmp(argv[i], "--id-prefix")){
			if(strlen(id) != 0){
				fprintf(stderr, "Error: -i and -I argument cannot be used together.\n\n");
				mqtt_print_usage();
				return 1;
			}
			if(i==argc-1){
				fprintf(stderr, "Error: -I argument given but no id prefix specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				if (strlen(argv[i+1]) >= MOSQ_MQTT_ID_MAX_LENGTH) {
					fprintf(stderr, "Error: max length of client id is %d.\n\n", MOSQ_MQTT_ID_MAX_LENGTH);
					mqtt_print_usage();
				} else {
					strcpy(id_prefix, argv[i+1]);
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-k") || !strcmp(argv[i], "--keepalive")){
			if(i==argc-1){
				fprintf(stderr, "Error: -k argument given but no keepalive specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				keepalive = atoi(argv[i+1]);
				if(keepalive>65535){
					fprintf(stderr, "Error: Invalid keepalive given: %d\n", keepalive);
					mqtt_print_usage();
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "-q") || !strcmp(argv[i], "--qos")){
			if(i==argc-1){
				fprintf(stderr, "Error: -q argument given but no QoS specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				ud->topic_qos = atoi(argv[i+1]);
				if(ud->topic_qos<0 || ud->topic_qos>2){
					fprintf(stderr, "Error: Invalid QoS given: %d\n", ud->topic_qos);
					mqtt_print_usage();
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "--quiet")){
			ud->quiet = true;
		}else if(!strcmp(argv[i], "-R")){
			ud->no_retain = true;
		}else if(!strcmp(argv[i], "-t") || !strcmp(argv[i], "--topic")){
			if(i==argc-1){
				fprintf(stderr, "Error: -t argument given but no topic specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				ud->topic_count++;
				ud->topics = realloc(ud->topics, ud->topic_count*sizeof(char *));
				ud->topics[ud->topic_count-1] = argv[i+1];
			}
			i++;
		}else if(!strcmp(argv[i], "-u") || !strcmp(argv[i], "--username")){
			if(i==argc-1){
				fprintf(stderr, "Error: -u argument given but no username specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				ud->username = argv[i+1];
			}
			i++;
		}else if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")){
			ud->verbose = 1;
		}else if(!strcmp(argv[i], "-P") || !strcmp(argv[i], "--pw")){
			if(i==argc-1){
				fprintf(stderr, "Error: -P argument given but no password specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				ud->password = argv[i+1];
			}
			i++;
		}else if(!strcmp(argv[i], "--will-payload")){
			if(i==argc-1){
				fprintf(stderr, "Error: --will-payload argument given but no will payload specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				will_payload = argv[i+1];
				will_payloadlen = strlen(will_payload);
			}
			i++;
		}else if(!strcmp(argv[i], "--will-qos")){
			if(i==argc-1){
				fprintf(stderr, "Error: --will-qos argument given but no will QoS specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				will_qos = atoi(argv[i+1]);
				if(will_qos < 0 || will_qos > 2){
					fprintf(stderr, "Error: Invalid will QoS %d.\n\n", will_qos);
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "--will-retain")){
			will_retain = true;
		}else if(!strcmp(argv[i], "--will-topic")){
			if(i==argc-1){
				fprintf(stderr, "Error: --will-topic argument given but no will topic specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				will_topic = argv[i+1];
			}
			i++;
		}else{
			fprintf(stderr, "Error: Unknown option '%s'.\n",argv[i]);
			mqtt_print_usage();
			return 1;
		}
	}

	/* verify necessary variable */
	if(clean_session == false && ((strlen(id_prefix) != 0) || (strlen(id) == 0))){
		if(!ud->quiet) fprintf(stderr, "Error: You must provide a client id if you are using the -c option.\n");
		return 1;
	}
	if(ud->topic_count == 0){
		fprintf(stderr, "Error: You must specify a topic to subscribe to.\n");
		mqtt_print_usage();
		return 1;
	}
	if(will_payload && !will_topic){
		fprintf(stderr, "Error: Will payload given, but no will topic given.\n");
		mqtt_print_usage();
		return 1;
	}
	if(will_retain && !will_topic){
		fprintf(stderr, "Error: Will retain given, but no will topic given.\n");
		mqtt_print_usage();
		return 1;
	}
	if(ud->password && !ud->username){
		if(!ud->quiet) fprintf(stderr, "Warning: Not using password since username not set.\n");
	}

	/* setup signal handler */
	signal(SIGINT, mqtt_signal_handler);
	signal(SIGTERM, mqtt_signal_handler);

	/* init mosquitto library */
	mosquitto_lib_init();

	/* setup client id */
	if(strlen(id_prefix) != 0){
		snprintf(id, sizeof(id), "%s%d", id_prefix, getpid());
	}else if(strlen(id) == 0){
		memset(hostname, '\0', sizeof(hostname));
		gethostname(hostname, sizeof(hostname));
		snprintf(id, sizeof(id), "mosqsub/%d-%s", getpid(), hostname);
	}
	if(strlen(id) > MOSQ_MQTT_ID_MAX_LENGTH){
		/* Enforce maximum client id length of 23 characters */
		id[MOSQ_MQTT_ID_MAX_LENGTH] = '\0';
	}

	/* start mosquitto */
	mosq = mosquitto_new(id, clean_session, ud);
	if(!mosq){
		if(!ud->quiet) fprintf(stderr, "Error: %s\n", strerror(errno));
		mosquitto_lib_cleanup();
		return 1;
	}

	/* setup mosquitto */
	if(debug){
		mosquitto_log_callback_set(mosq, mqtt_log_callback);
	}
	if(will_topic && mosquitto_will_set(mosq, will_topic, will_payloadlen, will_payload, will_qos, will_retain)){
		if(!ud->quiet) fprintf(stderr, "Error: Problem setting will.\n");
		mosquitto_lib_cleanup();
		return 1;
	}
	if(ud->username && mosquitto_username_pw_set(mosq, ud->username, ud->password)){
		if(!ud->quiet) fprintf(stderr, "Error: Problem setting username and password.\n");
		mosquitto_lib_cleanup();
		return 1;
	}
	mosquitto_connect_callback_set(mosq, mqtt_connect_callback);
	mosquitto_message_callback_set(mosq, mqtt_message_callback);
	if(debug){
		mosquitto_subscribe_callback_set(mosq, mqtt_subscribe_callback);
	}

	/* connect mosquitto */
	rc = mosquitto_connect(mosq, host, port, keepalive);
	if(rc){
		if(!ud->quiet){
			if(rc == MOSQ_ERR_ERRNO){
#ifndef WIN32
				strerror_r(errno, err, sizeof(err));
#else
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errno, 0, (LPTSTR)&err, sizeof(err), NULL);
#endif
				fprintf(stderr, "Error: %s\n", err);
			}else{
				fprintf(stderr, "Unable to connect (%d: %s).\n", rc, mosquitto_strerror(rc));
			}
		}
		mosquitto_lib_cleanup();
		return rc;
	}

	/*
	 * loop mosquitto,
	 * it use select() to call back the callback-function which defined before.
	 */
	while (run) {
		rc = mosquitto_loop(mosq, 1000, 1);
		if (run && rc) {
			fprintf(stderr, "MQTT: reconnect to server\n");
			mosquitto_reconnect(mosq);
		}
	}

	/* free mosquitto */
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	mqtt_userdata_free(ud);

	return 0;
}

