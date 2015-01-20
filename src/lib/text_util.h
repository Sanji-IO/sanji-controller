/**
 * @file text_util.h
 * @brief Declaration of text utility functions
 * @date 2012-10-13
 * @version 1.0.0
 */

#ifndef _TEXT_UTIL_H
#define _TEXT_UTIL_H

#ifdef  __cplusplus
extern "C" {
#endif

#define HEX2INT(v,x) \
{\
	if (x>='0' && x<='9') v = x-'0';\
	else if (x>='A' && x<='F') v = x-'A'+10;\
	else if (x>='a' && x<='f') v = x-'a'+10;\
	else v=0; \
}

extern void hex2bin(char *hex, int len, unsigned char *bin);
extern void bin2hex(unsigned char *bin, int len, char *hex);
extern void remove_trailing_spaces(char *str);
void remove_chars(char *str, char remove_char);
void remove_spaces(char *str);
void remove_newlines(char *str);
void remove_quotations(char *str);

#ifdef  __cplusplus
}
#endif

#endif  /* _TEXT_UTIL_H */
