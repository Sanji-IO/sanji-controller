#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#define MQTT_BUFSIZE 1024
/* message mode */
#define MQTT_MSGMODE_NONE 0
#define MQTT_MSGMODE_CMD 1
#define MQTT_MSGMODE_STDIN_LINE 2
#define MQTT_MSGMODE_STDIN_FILE 3
#define MQTT_MSGMODE_FILE 4
#define MQTT_MSGMODE_NULL 5
/* status */
#define MQTT_STATUS_CONNECTING 0
#define MQTT_STATUS_CONNACK_RECVD 1
#define MQTT_STATUS_WAITING 2

/*
 * This struct is used to pass data to callbacks.
 * An instance "ud" is created in main() and populated, then passed to
 * mosquitto_new().
 */
struct mqtt_userdata {
	char *topic;
	char *message;
	long msglen;
	int qos;
	int retain;
	int mode;
	int status;
	int mid_sent;
	int last_mid;
	bool connected;
	char *username;
	char *password;
	bool disconnect_sent;
	bool quiet;
};


/*
 * ##########################
 * Callback Functions
 * ##########################
 */
void mqtt_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	int rc = MOSQ_ERR_SUCCESS;
	struct mqtt_userdata *ud = NULL;

	assert(obj);
	ud = (struct mqtt_userdata *)obj;

	if(!result){
		switch(ud->mode){
			case MQTT_MSGMODE_CMD:
			case MQTT_MSGMODE_FILE:
			case MQTT_MSGMODE_STDIN_FILE:
				rc = mosquitto_publish(mosq, &ud->mid_sent, ud->topic, ud->msglen, ud->message, ud->qos, ud->retain);
				break;
			case MQTT_MSGMODE_NULL:
				rc = mosquitto_publish(mosq, &ud->mid_sent, ud->topic, 0, NULL, ud->qos, ud->retain);
				break;
			case MQTT_MSGMODE_STDIN_LINE:
				ud->status = MQTT_STATUS_CONNACK_RECVD;
				break;
		}
		if(rc){
			if(!ud->quiet){
				switch(rc){
					case MOSQ_ERR_INVAL:
						fprintf(stderr, "Error: Invalid input. Does your topic contain '+' or '#'?\n");
						break;
					case MOSQ_ERR_NOMEM:
						fprintf(stderr, "Error: Out of memory when trying to publish message.\n");
						break;
					case MOSQ_ERR_NO_CONN:
						fprintf(stderr, "Error: Client not connected when trying to publish.\n");
						break;
					case MOSQ_ERR_PROTOCOL:
						fprintf(stderr, "Error: Protocol error when communicating with broker.\n");
						break;
					case MOSQ_ERR_PAYLOAD_SIZE:
						fprintf(stderr, "Error: Message payload is too large.\n");
						break;
				}
			}
			mosquitto_disconnect(mosq);
		}
	}else{
		if(result && !ud->quiet){
			fprintf(stderr, "%s\n", mosquitto_connack_string(result));
		}
	}
}

void mqtt_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	struct mqtt_userdata *ud = NULL;

	assert(obj);
	ud = (struct mqtt_userdata *)obj;

	ud->connected = false;
}

void mqtt_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
	struct mqtt_userdata *ud = NULL;

	assert(obj);
	ud = (struct mqtt_userdata *)obj;

	if(ud->mode == MQTT_MSGMODE_STDIN_LINE){
		if(mid == ud->last_mid){
			mosquitto_disconnect(mosq);
			ud->disconnect_sent = true;
		}
	}else if(ud->disconnect_sent == false){
		mosquitto_disconnect(mosq);
		ud->disconnect_sent = true;
	}
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
int mqtt_load_stdin(void *obj)
{
	long pos = 0, rlen;
	char buf[MQTT_BUFSIZE];
	struct mqtt_userdata *ud = NULL;

	assert(obj);
	ud = (struct mqtt_userdata *)obj;

	ud->mode = MQTT_MSGMODE_STDIN_FILE;

	while(!feof(stdin)){
		rlen = fread(buf, 1, sizeof(buf), stdin);
		ud->message = realloc(ud->message, pos+rlen);
		if(!ud->message){
			if(!ud->quiet) fprintf(stderr, "Error: Out of memory.\n");
			return 1;
		}
		memcpy(&(ud->message[pos]), buf, rlen);
		pos += rlen;
	}
	ud->msglen = pos;

	if(!ud->msglen){
		if(!ud->quiet) fprintf(stderr, "Error: Zero length input.\n");
		return 1;
	}

	return 0;
}

int mqtt_load_file(const char *filename, void *obj)
{
	long pos, rlen;
	FILE *fptr = NULL;
	struct mqtt_userdata *ud = NULL;

	assert(obj);
	ud = (struct mqtt_userdata *)obj;

	fptr = fopen(filename, "rb");
	if(!fptr){
		if(!ud->quiet) fprintf(stderr, "Error: Unable to open file \"%s\".\n", filename);
		return 1;
	}
	ud->mode = MQTT_MSGMODE_FILE;
	fseek(fptr, 0, SEEK_END);
	ud->msglen = ftell(fptr);
	if(ud->msglen > 268435455){
		fclose(fptr);
		if(!ud->quiet) fprintf(stderr, "Error: File \"%s\" is too large (>268,435,455 bytes).\n", filename);
		return 1;
	}else if(ud->msglen == 0){
		fclose(fptr);
		if(!ud->quiet) fprintf(stderr, "Error: File \"%s\" is empty.\n", filename);
		return 1;
	}else if(ud->msglen < 0){
		fclose(fptr);
		if(!ud->quiet) fprintf(stderr, "Error: Unable to determine size of file \"%s\".\n", filename);
		return 1;
	}
	fseek(fptr, 0, SEEK_SET);
	ud->message = malloc(ud->msglen);
	if(!ud->message){
		fclose(fptr);
		if(!ud->quiet) fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}
	pos = 0;
	while(pos < ud->msglen){
		rlen = fread(&(ud->message[pos]), sizeof(char), ud->msglen-pos, fptr);
		pos += rlen;
	}
	fclose(fptr);
	return 0;
}

void mqtt_userdata_free(struct mqtt_userdata *ud)
{
	if (ud) {
		if(ud->message && ((ud->mode == MQTT_MSGMODE_FILE) || (ud->mode == MQTT_MSGMODE_STDIN_FILE))){
			free(ud->message);
		}

		free(ud);
	}
}

void mqtt_print_usage(void)
{
	int major, minor, revision;

	mosquitto_lib_version(&major, &minor, &revision);
	printf("mosquitto_pub is a simple mqtt client that will publish a message on a single topic and exit.\n");
	printf("mosquitto_pub version %s running on libmosquitto %d.%d.%d.\n\n", MQTT_VERSION, major, minor, revision);
	printf("Usage: mosquitto_pub [-h host] [-p port] [-q qos] [-r] {-f file | -l | -n | -m message} -t topic\n");
	printf("                     [-i id] [-I id_prefix]\n");
	printf("                     [-d] [--quiet]\n");
	printf("                     [-M max_inflight]\n");
	printf("                     [-u username [-P password]]\n");
	printf("                     [--will-topic [--will-payload payload] [--will-qos qos] [--will-retain]]\n");
	printf("       mosquitto_pub --help\n\n");
	printf(" -d : enable debug messages.\n");
	printf(" -f : send the contents of a file as the message.\n");
	printf(" -h : mqtt host to connect to. Defaults to localhost.\n");
	printf(" -i : id to use for this client. Defaults to mosquitto_pub_ appended with the process id.\n");
	printf(" -I : define the client id as id_prefix appended with the process id. Useful for when the\n");
	printf("      broker is using the clientid_prefixes option.\n");
	printf(" -l : read messages from stdin, sending a separate message for each line.\n");
	printf(" -m : message payload to send.\n");
	printf(" -M : the maximum inflight messages for QoS 1/2..\n");
	printf(" -n : send a null (zero length) message.\n");
	printf(" -p : network port to connect to. Defaults to 1883.\n");
	printf(" -q : quality of service level to use for all messages. Defaults to 0.\n");
	printf(" -r : message should be retained.\n");
	printf(" -s : read message from stdin, sending the entire input as a message.\n");
	printf(" -t : mqtt topic to publish to.\n");
	printf(" -u : provide a username (requires MQTT 3.1 broker)\n");
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
	unsigned int max_inflight = 20;
	bool debug = false;
	char buf[MQTT_BUFSIZE];
	char err[MQTT_ERR_BUFSIZE];
	/* client id */
	char id[MQTT_ID_LEN];
	char id_prefix[MQTT_ID_LEN];
	char hostname[MQTT_HOSTNAME_BUFSIZE];
	/* broker variable */
	char host[MQTT_IP_LEN] = "127.0.0.1";
	int port = 1883;
	int keepalive = 60;
	/* will information */
	char *will_topic = NULL;
	long will_payloadlen = 0;
	char *will_payload = NULL;
	int will_qos = 0;
	bool will_retain = false;
	/* temp variable */
	int i;
	int rc;
	int rc2;

	/* initialized program and user data structure */
	ud = malloc(sizeof(struct mqtt_userdata));
	memset(ud, 0, sizeof(struct mqtt_userdata));
	ud->last_mid = -1;
	ud->connected = true;
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
		}else if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")){
			debug = true;
		}else if(!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file")){
			if(ud->mode != MQTT_MSGMODE_NONE){
				fprintf(stderr, "Error: Only one type of message can be sent at once.\n\n");
				mqtt_print_usage();
				return 1;
			}else if(i==argc-1){
				fprintf(stderr, "Error: -f argument given but no file specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				if(mqtt_load_file(argv[i+1], ud)) return 1;
			}
			i++;
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
		}else if(!strcmp(argv[i], "-l") || !strcmp(argv[i], "--stdin-line")){
			if(ud->mode != MQTT_MSGMODE_NONE){
				fprintf(stderr, "Error: Only one type of message can be sent at once.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				ud->mode = MQTT_MSGMODE_STDIN_LINE;
			}
		}else if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--message")){
			if(ud->mode != MQTT_MSGMODE_NONE){
				fprintf(stderr, "Error: Only one type of message can be sent at once.\n\n");
				mqtt_print_usage();
				return 1;
			}else if(i==argc-1){
				fprintf(stderr, "Error: -m argument given but no message specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				ud->message = argv[i+1];
				ud->msglen = strlen(ud->message);
				ud->mode = MQTT_MSGMODE_CMD;
			}
			i++;
		}else if(!strcmp(argv[i], "-M")){
			if(i==argc-1){
				fprintf(stderr, "Error: -M argument given but max_inflight not specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				max_inflight = atoi(argv[i+1]);
			}
			i++;
		}else if(!strcmp(argv[i], "-n") || !strcmp(argv[i], "--null-message")){
			if(ud->mode != MQTT_MSGMODE_NONE){
				fprintf(stderr, "Error: Only one type of message can be sent at once.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				ud->mode = MQTT_MSGMODE_NULL;
			}
		}else if(!strcmp(argv[i], "-q") || !strcmp(argv[i], "--qos")){
			if(i==argc-1){
				fprintf(stderr, "Error: -q argument given but no QoS specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				ud->qos = atoi(argv[i+1]);
				if(ud->qos<0 || ud->qos>2){
					fprintf(stderr, "Error: Invalid QoS given: %d\n", ud->qos);
					mqtt_print_usage();
					return 1;
				}
			}
			i++;
		}else if(!strcmp(argv[i], "--quiet")){
			ud->quiet = true;
		}else if(!strcmp(argv[i], "-r") || !strcmp(argv[i], "--retain")){
			ud->retain = 1;
		}else if(!strcmp(argv[i], "-s") || !strcmp(argv[i], "--stdin-file")){
			if(ud->mode != MQTT_MSGMODE_NONE){
				fprintf(stderr, "Error: Only one type of message can be sent at once.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				if(mqtt_load_stdin(ud)) return 1;
			}
		}else if(!strcmp(argv[i], "-t") || !strcmp(argv[i], "--topic")){
			if(i==argc-1){
				fprintf(stderr, "Error: -t argument given but no topic specified.\n\n");
				mqtt_print_usage();
				return 1;
			}else{
				ud->topic = argv[i+1];
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
	if(!ud->topic || ud->mode == MQTT_MSGMODE_NONE){
		fprintf(stderr, "Error: Both topic and message must be supplied.\n");
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

	/* init mosquitto library */
	mosquitto_lib_init();

	/* setup client id */
	if(strlen(id_prefix) != 0){
		snprintf(id, sizeof(id), "%s%d", id_prefix, getpid());
	}else if(strlen(id) == 0){
		memset(hostname, '\0', sizeof(hostname));
		gethostname(hostname, sizeof(hostname));
		snprintf(id, sizeof(id), "mosqub/%d-%s", getpid(), hostname);
	}
	if(strlen(id) > MOSQ_MQTT_ID_MAX_LENGTH){
		/* Enforce maximum client id length of 23 characters */
		id[MOSQ_MQTT_ID_MAX_LENGTH] = '\0';
	}

	/* start mosquitto */
	mosq = mosquitto_new(id, true, ud);
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
	mosquitto_max_inflight_messages_set(mosq, max_inflight);
	mosquitto_connect_callback_set(mosq, mqtt_connect_callback);
	mosquitto_disconnect_callback_set(mosq, mqtt_disconnect_callback);
	mosquitto_publish_callback_set(mosq, mqtt_publish_callback);

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

	/* publish mosquitto mqtt message BASED ON DIFFERENT message mode */
	if(ud->mode == MQTT_MSGMODE_STDIN_LINE){
		mosquitto_loop_start(mosq);
	}

	do{
		if(ud->mode == MQTT_MSGMODE_STDIN_LINE){
			if(ud->status == MQTT_STATUS_CONNACK_RECVD){
				if(fgets(buf, sizeof(buf), stdin)){
					buf[strlen(buf)-1] = '\0';
					rc2 = mosquitto_publish(mosq, &ud->mid_sent, ud->topic, strlen(buf), buf, ud->qos, ud->retain);
					if(rc2){
						if(!ud->quiet) fprintf(stderr, "Error: Publish returned %d, disconnecting.\n", rc2);
						mosquitto_disconnect(mosq);
					}
				}else if(feof(stdin)){
					ud->last_mid = ud->mid_sent;
					ud->status = MQTT_STATUS_WAITING;
				}
			}else if(ud->status == MQTT_STATUS_WAITING){
#ifdef WIN32
				Sleep(1000);
#else
				usleep(1000000);
#endif
			}
			rc = MOSQ_ERR_SUCCESS;
		}else{
			rc = mosquitto_loop(mosq, -1, 1);
		}
	}while(rc == MOSQ_ERR_SUCCESS && ud->connected);

	if(ud->mode == MQTT_MSGMODE_STDIN_LINE){
		mosquitto_loop_stop(mosq, false);
	}

	/* free mosquitto */
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	mqtt_userdata_free(ud);

	return rc;
}
