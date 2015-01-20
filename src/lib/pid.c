/**
 * @file pic.c
 * @brief Declaration of pid utility
 * @date 2014-08-28
 * @version 1.0.0
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>

/**
 * @brief change access and modification time of PID file
 * @param pidfile PID filename
 * @return 0:OK, others: failure
 */
int
touch_pid(char *pidfile)
{
	return utime(pidfile, NULL);
}

/**
 * @brief Create PID file as lock file
 * @param pidfile PID filename
 * @return 0:OK, others: failure
 */
int
create_pid(char *pidfile)
{
	FILE *fp;
	int fd;

	if (!pidfile) {
		printf("create PID file failed\n");
		return -1;
	}

	/*
	 * unlink the pid_file, if it exists, prior to open. Without doing this
	 * the open will fail if the user specified pid_file already exists.
	 */
	unlink(pidfile);
	fd = open(pidfile, O_CREAT | O_EXCL | O_WRONLY, 0600);
	if (fd == -1) {
		printf("create PID file failed\n");
		return -1;
	}

	if ((fp = fdopen(fd, "w")) == NULL) {
		printf("create PID file failed\n");
		return -1;
	}

	fprintf(fp, "%d\n", (int)getpid());

	fclose(fp);
	close(fd);

	return 0;
}

