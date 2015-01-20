/**
 * @file time_util.h
 * @brief Declaration of time utility functions
 * @date 2014-09-04
 * @version 1.0.0
 */

#ifndef _TIME_UTIL_H
#define _TIME_UTIL_H

#ifdef  __cplusplus
extern "C" {
#endif

/* defintion of timestamp */
#define TIMESTAMP_LEN 32
#define TIMESTAMP_MODE_MONOTONIC 0
#define TIMESTAMP_MODE_UNIXTIME 1
#define TIMESTAMP_MODE_DATETIME 2

extern int get_today(char *today);
extern void get_timestamp(int mode, char *timestamp, unsigned int size);

#ifdef  __cplusplus
}
#endif

#endif  /* _TIME_UTIL_H */
