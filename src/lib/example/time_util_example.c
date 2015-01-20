#include <stdio.h>
#include <string.h>
#include "time_util.h"

int main()
{
	char today[16];
	char timestamp[TIMESTAMP_LEN];

	memset(today, '\0', sizeof(today));

	if (get_today(today)) return 1;
	printf("today is '%s'\n", today);

	get_timestamp(TIMESTAMP_MODE_MONOTONIC, timestamp, TIMESTAMP_LEN);
	printf("timestamp is '%s'\n", timestamp);
	get_timestamp(TIMESTAMP_MODE_UNIXTIME, timestamp, TIMESTAMP_LEN);
	printf("timestamp is '%s'\n", timestamp);
	get_timestamp(TIMESTAMP_MODE_DATETIME, timestamp, TIMESTAMP_LEN);
	printf("timestamp is '%s'\n", timestamp);

	return 0;
}
