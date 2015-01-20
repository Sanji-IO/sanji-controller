/**
 * @file typedefs.h
 * @brief Define data type for meter data
 * @date 2012-10-05
 * @version 1.0.0
 */

#ifndef _TYPEDEFS_H
#define _TYPEDEFS_H

#ifndef CSTD
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef char				CHAR;
typedef char				DIGIT;
typedef unsigned char		BYTE;
typedef unsigned char		BCD;
typedef unsigned short		WORD;

#ifndef CSTD
typedef uint8_t		UINT8;
typedef uint16_t	UINT16;
typedef uint32_t	UINT32;
typedef uint32_t	NUMBER;
typedef uint64_t	UINT64;
#else
typedef unsigned char		UINT8;
typedef unsigned short		UINT16;
typedef unsigned int		UINT32;
typedef unsigned long long	UINT64;
typedef unsigned long long	NUMBER;
#endif

typedef struct _DATETIME {
	BCD yr;		/* 00 ~ 99 */
	BCD mon;	/* 01 ~ 12 */
	BCD mday;	/* 01 ~ 31 */
	BCD hr;		/* 00 ~ 23 */
	BCD min;	/* 00 ~ 59 */
	BCD sec;	/* 00 ~ 59 */
} __attribute__((packed)) DATETIME;

#ifdef __cplusplus
}
#endif

#endif /* _TYPEDEFS_H */
