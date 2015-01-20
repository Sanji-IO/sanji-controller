/**
 * @file strext.h
 * @brief Declaration of string extesion functions
 * @date 2014-09-03
 * @version 1.0.0
 */
#ifndef	_STREXT_H
#define	_STREXT_H

#ifdef  __cplusplus
extern "C" {
#endif

#define MAX_PATH	260

char * xstrdup(char *str);
char * strlwr(char *str);
char * strupr(char *str);
char * strtrim(char *str, const char *trim);
char * strrtrim(char *str, const char *trim);
char * strltrim(char *str, const char *trim);

#ifdef  __cplusplus
}
#endif

#endif /* _STREXT_H */
