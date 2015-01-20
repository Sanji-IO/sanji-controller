/**
 * @file random_util.h
 * @brief Declaration of random utility
 * @date 2014-09-04
 * @version 1.0.0
 */

#ifndef _RANDOM_UTIL_H
#define _RANDOM_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/* mode of random number generator */
#define RAND_MODE_SEQ 0
#define RAND_MODE_RANDOM 1

int generate_random(int mode);

#ifdef __cplusplus
}
#endif

#endif /* _RANDOM_UTIL_H */
