/**
 * @file crc16.h
 * @brief Declaration of file lock utility
 * @date 2012-10-05
 * @version 1.0.0
 */

#ifndef _CRC16_H
#define _CRC16_H

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern UINT16 get_crc16(const char *pData, UINT32 length);
extern UINT16 do_crc16(UINT16 crc, const char *pData, UINT32 length);

#ifdef __cplusplus
}
#endif

#endif /* _CRC16_H */
