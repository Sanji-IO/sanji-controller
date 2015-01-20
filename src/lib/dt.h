/**
 * @file dt.h
 * @brief Declaration of datetime utility
 * @date 2012-10-18
 * @version 1.0.0
 */

#ifndef _DT_H
#define _DT_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "typedefs.h"

/*
 * ########################
 * LIBRARY FUNCTION
 * ########################
 */

extern void tm2dt(struct tm *, DATETIME *);
extern void dt2tm(DATETIME *, struct tm *);
extern void dt2str(DATETIME *, char *);
extern void tm2str(struct tm *, char *);


#ifdef  __cplusplus
}
#endif

#endif  /* _DT_H */
