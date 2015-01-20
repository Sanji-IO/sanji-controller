/**
 * @file ini.h
 * @brief Declaration of ini parser
 * @date 2014-09-02
 * @version 1.0.0
 */
#ifndef _INI_H
#define _INI_H

#ifdef  __cplusplus
extern "C" {
#endif

/* minimal ini item size */
#define INI_MINSIZE 64

#define INI_ERROR_NULL_POINTER      "null pointer"
#define INI_ERROR_OUT_OF_MEMORY     "memory not enough"
#define INI_ERROR_OPEN_FAILED       "file open failed: %s"
#define INI_ERROR_INVALID_SYNTAX    "syntax invalid: %s"

typedef struct _init_t {
	int num;            /** number of keys */
	int size;           /** storage size */
	char **key;         /** list of string keys */
	char **val;         /** list of string values */
	unsigned int *hash; /** list of hash values for keys */
} ini_t;

ini_t *ini_init(char *file);
void ini_release(ini_t *this);
int ini_findkey(ini_t *this, const char *section, char *key, char *result, size_t size);
int ini_parse_file(ini_t *this, char *file);
char *ini_get(ini_t *this, char *key);
int ini_set(ini_t *this, char *key, char *value);
int ini_print(ini_t *this);

#ifdef  __cplusplus
}
#endif

#endif /* _INI_H */

