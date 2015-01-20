#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "ini.h"
#include "strext.h"
#include "debug.h"

/* status of the ini line */
enum INI_LINE_STATUS {
	INI_LINE_UNKNOWN,
	INI_LINE_EMPTY, 
	INI_LINE_COMMENT,
	INI_LINE_SECTION,
	INI_LINE_VALUE,
	INI_LINE_ERROR = -1
};

/**
 * @brief reallocate the memory size
 * @param obj object pointer
 * @param current_size current size of the object
 * @param extend_size size after be extended
 * @param size slot size
 * @return error: NULL, success: new pointer
 */
static void *_extend_mem(void *obj, int current_size, int extend_size, size_t size)
{
	void *new = NULL;

	new = calloc(extend_size, size);
	if (!new) return NULL;

	memcpy(new, obj, current_size * size);
	free(obj);

	return new;
}

/**
 * @brief allocate or extend ini memory storage
 * @param this ini object pointer
 * @return error: 1,  success: 0
 */
static int ini_alloc(ini_t *this)
{
	int size;

	if (!this) return 1;

	/* allocate or extend the memory */
	size = this->size + INI_MINSIZE;
	this->key = (char **) _extend_mem(
			this->key, 
			this->size, 
			size, sizeof(char *));
	this->val = (char **) _extend_mem(
			this->val, 
			this->size, 
			size, sizeof(char *));
	this->hash = (unsigned int *) _extend_mem(
			this->hash, 
			this->size, 
			size, 
			sizeof(unsigned int));

	if (!this->key || !this->val || !this->hash) {
		DEBUG_PRINT(INI_ERROR_OUT_OF_MEMORY);
		return 1;
	}

	this->size = size;

	return 0;
}

/**
 * @brief calculate a hash value by the given key
 * @param this ini object pointer
 * @param key key name
 * @return hash value
 */
static unsigned int ini_hash(char *key)
{
	int i;
	int len;
	unsigned int hash;

	len = strlen(key);
	for (hash = 0, i = 0; i < len; i++) {
		hash += (unsigned int) key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

/**
 * @brief find the index of the given key
 * @param this ini object pointer
 * @param key key name
 * @return error: -1, success: index
 */
static int ini_find(ini_t *this, char *key)
{
	int i;
	unsigned int hash;

	if (!this || !key || this->num <= 0) return -1;

	/* hash for this key */
	hash = ini_hash(key);

	/* find duplicated value */
	for (i = 0; i < this->size; i++)
	{
		if (NULL == this->key[i]) continue;

		/* hash and key value are the same */
		if (hash == this->hash[i] && 0 == strcmp(key, this->key[i])) {
			return i;
		}
	}

	return -1;
}

/**
 * @brief add or update a key value
 *
 * ini_set function is used to add or update an ini key value in a ini object.
 * The format of the key should be 
 *	"<section>:<key>".
 *
 * @param this ini object pointer
 * @param key key name
 * @param value the key value
 * @return error: 1, success: 0
 */
int ini_set(ini_t *this, char *key, char *value)
{
	int i;
	unsigned int hash;

	if (!this || !key) {
		DEBUG_PRINT(INI_ERROR_NULL_POINTER);
		return 1;
	}

	/* hash for this key */
	hash = ini_hash(key);

	/* find duplicated value */
	i = ini_find(this, key);
	if (i >= 0) {
		/* modified the value */
		if (this->val[i]) free(this->val[i]);
		this->val[i] = value ? xstrdup(value) : NULL;

		return 0;
	}

	/* add a new value */
	if (this->num == this->size) {
		if (ini_alloc(this)) return 1;
	}

	/* find the first empty slot */
	for (i = 0; i < this->size; i++) {
		if (NULL == this->key[i]) break;
	}

	this->key[i] = xstrdup(key);
	this->val[i] = value ? xstrdup(value): NULL;
	this->hash[i] = hash;
	this->num++;

	return 0;
}

/**
 * @brief get a key value
 *
 * ini_get function is used to get a key value from an ini object.
 * The format of the key should be 
 *	"<section>:<key>".
 *
 * @param this ini object pointer
 * @param key the specified key name
 * @return error: NULL, success: value pointer
 */
char *ini_get(ini_t *this, char *key)
{
	int i;
	unsigned int hash;

	if (!this || !key) {
		DEBUG_PRINT(INI_ERROR_NULL_POINTER);
		return NULL;
	}

	hash = ini_hash(key);
	for (i = 0; i < this->size; i++) {
		if (NULL == this->key[i]) continue;

		if (hash == this->hash[i]) {
			if (0 == strcmp(key, this->key[i])) return this->val[i];
		}
	}

	return NULL;
}

/**
 * @brief parse one line of the ini file
 * @param line line to be parsed
 * @param section record the section name
 * @param key record the key name
 * @param value record the key value
 * @return success:0
 */
static int ini_parse_line(char *line, char *section, char *key, char *value)
{
	int status = INI_LINE_UNKNOWN;
	int len;
	char temp[MAX_PATH] = { 0 };

	strncpy(temp, strtrim(line, NULL), sizeof(temp));
	len = strlen(temp);

	if (len < 1) {
		status = INI_LINE_EMPTY;
	} else if ('#' == line[0] || ';' == line[0]) {
		status = INI_LINE_COMMENT;
	} else if ('[' == line[0] && ']' == line[len - 1]) {
		/* [section] */
		status = INI_LINE_SECTION;
		sscanf(line, "[%[^]]", section);
		strtrim(section, NULL);
	} else if (2 == sscanf(line, "%[^=] = \"%[^\"]\"", key, value)
              || 2 == sscanf(line, "%[^=] = '%[^\']'", key, value)
              || 2 == sscanf(line, "%[^=] = %[^;#]", key, value)) {
		status = INI_LINE_VALUE;
		/* key = value */
		strtrim(key, NULL);
		strtrim(value, NULL);

		/* for empty value of '' or "" */
		if (!strcmp(value, "\"\"") || (!strcmp(value, "''")))
			value[0] = 0;
	} else if (2 == sscanf(line, "%[^=] = %[;#]", key, value)
              || 2 == sscanf(line, "%[^=] %[=]", key, value)) {
		/* special cases:
		 * key = 
		 * key = ;
		 * key = #
		 */
		status = INI_LINE_VALUE;
		strtrim(key, NULL);
		value[0] = 0;
	} else {
		status = INI_LINE_ERROR;
	}

	return status;
}

/**
 * @brief initialize the ini object
 *
 * ini_parse_file function used to initialize the memory and load
 * the ini file into prepared memory.
 *
 * @param this ini object pointer
 * @param file ini file path
 * @return success: ini object pointer
 */
int ini_parse_file(ini_t *this, char *file)
{
	FILE *in = NULL;
	int len = 0;
	int last = 0;
	char line[MAX_PATH];
	char section[MAX_PATH];
	char sec_key[MAX_PATH];
	char key[MAX_PATH];
	char val[MAX_PATH];

	/* verify arguments */
	if (!this || !file) return 1;

	/* open the ini file to parse */
	in = fopen(file, "r");
	if (!in) {
		DEBUG_PRINT(INI_ERROR_OPEN_FAILED, file);
		return 1;
	}

	/* read line by line */
	while (fgets(line + last, sizeof(line) - last, in))
	{
		len = strlen(line) - 1;
		if (0 == len) continue;

		/* discard \n and spaces at end the line */
		while ((len > 0) && ('\n' == line[len] || isspace(line[len]))) {
			line[len--] = 0;
		}

		/* multi-line value */
		if ('\\' == line[len]) {
			last = len;
			continue;
		}
		last = 0;

		switch (ini_parse_line(line, section, key, val)) {
		case INI_LINE_SECTION:
			if (ini_set(this, section, NULL)) {
				fclose(in);
				return 1;
			}
			break;

		case INI_LINE_VALUE:
			snprintf(sec_key, sizeof(sec_key), "%s:%s", section, key);
			if (ini_set(this, sec_key, val)) {
				fclose(in);
				return 1;
			}
			break;

		case INI_LINE_ERROR:
			DEBUG_PRINT(INI_ERROR_INVALID_SYNTAX, line);
			break;

		case INI_LINE_EMPTY:
		case INI_LINE_COMMENT:
		default:
			break;
		}
		memset(line, 0, sizeof(line));
	}

	/* close input file */
	fclose(in);

	return 0;
}

/**
 * @brief print out the whole ini file
 *
 * ini_print function used to print the whole ini content.
 *
 * @param this ini object pointer
 * @return success:0
 */
int ini_print(ini_t *this)
{
	int i;
	int num;

	num = this->num;
	printf("num: %d\n", num);
	for (i = 0; i < this->size && num > 0; i++) {
		if (this->key[i]) {
			if (strchr(this->key[i], ':')) {
				printf("\"%s\" = \"%s\"\n", this->key[i], this->val[i] ? (char *) this->val[i] : "");
			} else {
				printf("[%s]\n", this->key[i]);
			}
		}
		num--;
	}

	return 0;
}

/** 
 * @brief ini object construction
 *
 * ini_init function used to construct the ini object.
 *
 * @param ini ini file path
 * @return error: NULL, success: pointer 
 */
ini_t *ini_init(char *file)
{
	ini_t *this = NULL;

	/* initialize the ini object */
	this = (ini_t *)calloc(1, sizeof(ini_t));
	if (!this || ini_alloc(this)) {
		DEBUG_PRINT(INI_ERROR_OUT_OF_MEMORY);
		ini_release(this);
		return NULL;
	}

	/* init instant */
	if (ini_parse_file(this, file)) {
		ini_release(this);
		return NULL;
	}

	return this;
}

/**
 * @brief free the ini contents (key/value/hash)
 *
 * ini_release function used to release the memory
 *
 * @param this ini object pointer
 * @return success:0
 */
void ini_release(ini_t *this)
{
	int i;
	int num;

	if (!this) return;

	num = this->num;
	for (i = 0; i < this->size && num > 0; i++) {
		if (this->key[i]) free(this->key[i]);
		if (this->val[i]) free(this->val[i]);
		num--;
	}
	free(this->key);
	free(this->val);
	free(this->hash);
	free(this);
}

/** 
 * @brief find value with a specified section and key from ini object
 * 
 * ini_findkey function used to read the value according to section and key. 
 *
 * @param this ini object instant
 * @param section specified section name
 * @param key specified key names
 * @param result buffer to store the value
 * @param size size of the app_names
 * @return error: 1, success: 0
 */
int ini_findkey(ini_t *this, const char *section, char *key, char *result, size_t size)
{
	char tmp[1024] = { 0 };
	char *value;

	if (!this || !section || !key) return -1;

	/* format the key */
	snprintf(tmp, sizeof(tmp), "%s:%s", section, key);
	strtrim(tmp, NULL);

	memset(result, 0, size);
	value = ini_get(this, tmp);
	if (!value) return 1;

	strncpy(result, value, size);

	return 0;
}

