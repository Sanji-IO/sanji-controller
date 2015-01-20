/**
 * @file bswap.h
 * @brief Declaration of byte swap macro
 * @date 2012-10-13
 * @version 1.0.0
 */

#ifndef _BSWAP_H
#define _BSWAP_H

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * ########################
 * MACRO
 * ########################
 */

#if WORDS_BIGENDIAN
#define BSWAP16(c) c
#define BSWAP32(c) c
#else
/* convert the bytes of a 16-bit integer to big endian */
#define BSWAP16(c) (((((c) & 0xff) << 8) | (((c) >> 8) & 0xff)))
/* convert the bytes of a 32-bit integer to big endian */
#define BSWAP32(c) ((((c)>>24)&0xff)|(((c)>>8)&0xff00)|(((c)<<8)&0xff0000)|((c)<<24))
#endif

#ifdef  __cplusplus
}
#endif

#endif  /* _BSWAP_H */
