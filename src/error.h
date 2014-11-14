#ifndef _SANJI_ERROR_H_
#define _SANJI_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif


/*
 * SANJI Error Codes
 */
#define SANJI_INTERNAL_ERROR	(-2)	// SERVER ERROR: out of memory, unknown crash
#define SANJI_DATA_ERROR		(-1)	// DATA ERROR: invalid data
#define SANJI_SUCCESS			0
#define SANJI_LOCKED			1
#define SANJI_NOT_FOUND			2

#ifdef __cplusplus
}
#endif

#endif  /* _SANJI_HTTP_H_ */
