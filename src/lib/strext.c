/**
 * @file strext.c
 * @brief Definition of string extesion functions
 * @date 2014-09-03
 * @version 1.0.0
 */
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "strext.h"

/**
 * @brief allocate a memory space for string
 * @param str string
 * @return error: NULL, success: pointer
 */
char *xstrdup(char *str)
{
	char *dest = NULL;

	if (!str) return NULL;

	dest = calloc(strlen(str) + 1, sizeof(char));
	if (dest) strcpy(dest, str);

	return dest;
}

/**
 * @brief trim the unrequired characters
 * @param str string
 * @param trim characters to be trim
 * @return error: NULL, success: pointer
 */
char *strtrim(char *str, const char *trim)
{
	return strltrim(strrtrim(str, trim), trim);
}

/**
 * @brief trim the unrequired characters from right side
 * @param str string
 * @param trim characters to be trim
 * @return error: NULL, success: pointer
 */
char *strrtrim(char *str, const char *trim)
{
	char *end = NULL;

	if (!str) return NULL;

	if (!trim)
		trim = " \t\n\r";
		
	end = str + strlen(str);
	while (end-- > str) {
		if (!strchr(trim, *end)) return str;
		*end = 0;
	}

	return str;
}

/**
 * @brief trim the unrequired characters from left side
 * @param str string
 * @param trim characters to be trim
 * @return error: NULL, success: pointer
 */
char *strltrim(char *str, const char *trim)
{
	char *head = str;

	if (!str) return NULL;
	
	if (!trim) trim = " \t\r\n";
	
	while (*str) {
		if (!strchr(trim, *str)) {
			memmove(head, str, strlen(str) + 1);
			return head;
		}
		*str = 0;
		str++;
	}

	return head;
}

/**
 * @brief convert the string to lower case
 * @param str string
 * @return error: NULL, success: pointer
 */
char *strlwr(char *str)
{
	char *head = str;

	if (!str) return NULL;
		
	while ((*str = (char)tolower(*str))) {
		++str;
	}
	
	return head;
}

/**
 * @brief convert the string to upper case
 * @param str string
 * @return error: NULL, success: pointer
 */
char *strupr(char *str)
{
	char *head = str;

	if (!str) return NULL;
		
	while ((*str = (char)toupper(*str))) {
		++str;
	}

	return head;
}

