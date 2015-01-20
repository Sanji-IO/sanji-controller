#include <stdio.h>
#include <unistd.h>
#include "pid.h"

#define PID_FILE "/tmp/pid_file"

int main()
{
	if (create_pid(PID_FILE)) return -1;

	printf("program start!\n");

	sleep(10);

	return 0;
}
