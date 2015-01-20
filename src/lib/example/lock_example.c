#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lock.h"

#define LOCK_FILE "/tmp/lock_file"

int main()
{
	int fd;
	int pid;

	fd = open(LOCK_FILE, O_WRONLY | O_CREAT);

	while (1) {
		pid = is_write_lockable(fd, 0, SEEK_SET, 0);
		if (!pid) break;
		printf("File is locked by pid(%d)\n", pid);
		sleep(1);
	}

	write_lock(fd, 0, SEEK_SET, 0);
	printf("Lock!\n");

	sleep(10);

	write_unlock(fd, 0, SEEK_SET, 0);
	printf("Unlock!\n");

	close(fd);

	return 0;
}
