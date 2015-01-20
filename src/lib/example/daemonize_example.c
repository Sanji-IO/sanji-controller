#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "daemonize.h"

static int run = 1;

void signal_handler()
{
	run = 0;
}

int main()
{
	int fd;
	int enable_stdio = 0;
	char str[64];

	printf("Before daemonization: pid(%d)\n", getpid());

	/* setup signal handler */
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);

	if (daemonize_process(enable_stdio)) {
		printf("Daemon start error!\n");
		exit(1);
	}

	/* open file */
	fd = open("/tmp/daemonization", O_WRONLY | O_CREAT);

	memset(str, '\0', sizeof(str));
	sprintf(str, "Finish daemonization: pid(%d)\n", getpid());
	fprintf(stderr, "Finish daemonization: pid(%d)\n", getpid());
	write(fd, str, strlen(str));
	fsync(fd);

	while (run) {
		memset(str, '\0', sizeof(str));
		sprintf(str, "I(%d) still alive.\n", getpid());
		fprintf(stderr, "I(%d) still alive.\n", getpid());
		write(fd, str, strlen(str));
		fsync(fd);
		sleep(3);
	}

	close(fd);
	unlink("/tmp/daemonization");

	return 0;
}
