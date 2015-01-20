/**
 * @file time_util.c
 * @brief Define time utility functions
 * @date 2012-12-25
 * @version 1.0.0
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "time_util.h"

/**
 * @brief get today's date string
 * @param today today's date string in format of YYYYMMDD
 * @return 0:OK, others: failure
 * @remarks parameter today must have enough space
 */
int
get_today(char *today)
{
	time_t now;
	struct tm *tm;

	/* get current time */
	now = time(NULL);
	if (now == (time_t)-1) {
		return -1;
	}

	tm = localtime(&now);
	sprintf(today, "%04d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);

	return 0;
}

#ifdef WIN32
static bool tick64 = false;
static void _windows_time_version_check(void)
{
	OSVERSIONINFO vi;
	tick64 = false;

	memset(&vi, 0, sizeof(OSVERSIONINFO));
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&vi)) {
		if (vi.dwMajorVersion > 5) {
			tick64 = true;
		}
	}
}
#endif

/**
 * @brief get timestamp
 * @param mode monotonic-time, unixtime or datetime
 * @param timestamp timestamp string
 * @param size size of input timestamp string
 */
void get_timestamp(int mode, char *timestamp, unsigned int size)
{
	struct timespec tp;
	time_t ltime;
	struct tm *tm_time;

	if (!timestamp || size < TIMESTAMP_LEN) return;

	memset(timestamp, 0, size);

	switch (mode) {
	case TIMESTAMP_MODE_MONOTONIC:
#ifdef WIN32
		_windows_time_version_check();
		if (tick64) {
			snprintf(timestamp, size, "%ld", GetTickCount64()/1000);
		} else {
			snprintf(timestamp, size, "%ld", GetTickCount()/1000); // FIXME: need to deal with overflow.
		}
#elif _POSIX_TIMERS>0 && defined(_POSIX_MONOTONIC_CLOCK)
		clock_gettime(CLOCK_MONOTONIC, &tp);
		snprintf(timestamp, size, "%ld", tp.tv_sec);
#else
		snprintf(timestamp, size, "%ld", time(NULL));
#endif
		break;

	case TIMESTAMP_MODE_UNIXTIME:
		snprintf(timestamp, size, "%ld", time(NULL));
		break;

	case TIMESTAMP_MODE_DATETIME:
	default:
		time(&ltime);
		tm_time = localtime(&ltime);
		snprintf(timestamp, size, "%04d%02d%02d%02d%02d%02d"
				, (1900 + tm_time->tm_year)
				, (1 + tm_time->tm_mon)
				, tm_time->tm_mday
				, tm_time->tm_hour
				, tm_time->tm_min
				, tm_time->tm_sec);
		break;
	}
}

