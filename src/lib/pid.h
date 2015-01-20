/**
 * @file pid.h
 * @brief Declaration of pid utility
 * @date 2014-08-28
 * @version 1.0.0
 */

#ifndef _PID_H
#define _PID_H

#ifdef  __cplusplus
extern "C" {
#endif

extern int create_pid(char *pidfile);
extern int touch_pid(char *pidfile);

#ifdef  __cplusplus
}
#endif

#endif  /* _PID_H */
