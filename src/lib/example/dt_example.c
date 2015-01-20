#include <stdio.h>
#include <time.h>
#include <string.h>
#include "dt.h"

int quit = 0;

int main()
{
	char timestamp[32];
	time_t ltime;
	struct tm *tm1, tm2;
	DATETIME dt;

	/* get linux datetime */
	time(&ltime);
	tm1 = localtime(&ltime);

	printf("Struct tm format:\n");
	printf("yr=%d\n", tm1->tm_year);
	printf("mon=%d\n", tm1->tm_mon);
	printf("mday=%d\n", tm1->tm_mday);
	printf("hr=%d\n", tm1->tm_hour);
	printf("min=%d\n", tm1->tm_min);
	printf("sec=%d\n", tm1->tm_sec);

	tm2dt(tm1, &dt);
	printf("DATEIME format:\n");
	printf("yr=%x\n", dt.yr);
	printf("mon=%x\n", dt.mon);
	printf("mday=%x\n", dt.mday);
	printf("hr=%x\n", dt.hr);
	printf("min=%x\n", dt.min);
	printf("sec=%x\n", dt.sec);

	dt2tm(&dt, &tm2);
	printf("Struct tm format:\n");
	printf("yr=%d\n", tm2.tm_year);
	printf("mon=%d\n", tm2.tm_mon);
	printf("mday=%d\n", tm2.tm_mday);
	printf("hr=%d\n", tm2.tm_hour);
	printf("min=%d\n", tm2.tm_min);
	printf("sec=%d\n", tm2.tm_sec);

	/* convert datetime value to string */
	memset(timestamp, '\0', sizeof(timestamp));
	tm2str(tm1, timestamp);
	printf("%s\n", timestamp);
	
	memset(timestamp, '\0', sizeof(timestamp));
	dt2str(&dt, timestamp);
	printf("%s\n", timestamp);
	
	return 0;
}
