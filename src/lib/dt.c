/**
 * @file dt.c
 * @brief datetime utility function
 * @date 2014-08-28
 * @version 1.0.0
 */
#include <stdio.h>
#include <time.h>
#include "typedefs.h"

static unsigned short
bcd2int(BCD b)
{
	return ((b >> 4) * 10) + (b & 0xF);
}

static BCD
int2bcd(unsigned short d)
{
	return ((d / 10) << 4) | (d % 10);
}

/**
 * @brief convert Linux datetime value to DATETIME value
 * @param tm OUT: Linux datetime value
 * @param dt DATETIME value
 */
void
tm2dt(struct tm *tm, DATETIME *dt)
{
	unsigned short yr = (tm->tm_year + 1900) % 100;
	dt->yr = int2bcd(yr);
	dt->mon = int2bcd(tm->tm_mon+1);
	dt->mday = int2bcd(tm->tm_mday);
	dt->hr = int2bcd(tm->tm_hour);
	dt->min = int2bcd(tm->tm_min);
	dt->sec = int2bcd(tm->tm_sec);
}

/**
 * @brief convert DATETIME value to Linux datetime value
 * @param dt DATETIME value
 * @param tm OUT: Linux datetime value
 */
void
dt2tm(DATETIME *dt, struct tm *tm)
{
	tm->tm_year = bcd2int(dt->yr) + 2000 - 1900;
	tm->tm_mon = bcd2int(dt->mon) - 1;
	tm->tm_mday = bcd2int(dt->mday);
	tm->tm_hour = bcd2int(dt->hr);
	tm->tm_min = bcd2int(dt->min);
	tm->tm_sec = bcd2int(dt->sec);
}

/**
 * @brief convert DATETIME value to string
 * @param dt DATETIME value
 * @param buf OUT: DATETIME value string
 * @remarks parameter buf must have enough space
 */
void
dt2str(DATETIME *dt, char *buf)
{
	sprintf(buf, "%04d%02d%02d%02d%02d%02d",
		bcd2int(dt->yr) + 2000,
		bcd2int(dt->mon),
		bcd2int(dt->mday),
		bcd2int(dt->hr),
		bcd2int(dt->min),
		bcd2int(dt->sec));
}

/**
 * @brief convert linux datetime value to string
 * @param tm linux datetime value
 * @param buf OUT: linux datetime value string
 * @remarks parameter buf must have enough space
 */
void
tm2str(struct tm *tm, char *buf)
{
	sprintf(buf, "%04d%02d%02d%02d%02d%02d",
		tm->tm_year + 1900,
		tm->tm_mon + 1,
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec);
}

