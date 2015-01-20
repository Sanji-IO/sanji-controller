/**
 * @file util.c
 * @brief Define text utility functions
 * @date 2012-10-15
 * @version 1.0.0
 */
#include <string.h>
#include "text_util.h"

/**
 * @brief convert HEX string into binary data
 * @param hex HEX string in size of double value of len
 * @param len length of binary data
 * @param bin binary data in size of value of len
 * @return none
 */
void
hex2bin(char *hex, int len, unsigned char *bin)
{
	int i;
	char *s = hex;
	unsigned char h, l;
	unsigned char *p = bin;

	for (i = 0; i < len; i++, p++, s += 2) {
		HEX2INT(h, s[0]);
		HEX2INT(l, s[1]);
		*p = (h<<4) + l;
	}
}

/**
 * @brief convert binary data into HEX string
 * @param bin binary data
 * @param len length of binary data
 * @param hex HEX string in size of double value of len
 * @return none
 */
void
bin2hex(unsigned char *bin, int len, char *hex)
{
	char *p = hex;
	unsigned char *s = bin;
	int i;
	unsigned char b;
	const char HexNumbers[] = "0123456789ABCDEF";

	for (i = 0; i < len; i++, s++) {
		b = *s;
		*p = HexNumbers[b >> 4]; p++;
		*p = HexNumbers[b & 0x0F]; p++;
	}
}

/**
 * @brief remove trailing space
 * @param str string
 * @return none
 */
void
remove_trailing_spaces(char *str)
{
	int len;
	int i;

	if (!str) return;

	len = strlen(str);

	for (i = len-1; i >= 0; i--) {
		if (str[i] != 0x20) break;
		str[i] = 0;
	}
}

/**
 * @brief remove characters from string
 * @param str string
 * @param remove_char characters
 * @return none
 */
void remove_chars(char *str, char remove_char)
{
	char *i = str;
	char *j = str;

	while (*j != 0) {
		*i = *j++;
		if (*i != remove_char) {
			i++;
		}
	}
	*i = 0;
}

/**
 * @brief remove spaces from string
 * @param str string
 * @return none
 */
void remove_spaces(char *str)
{
	remove_chars(str, ' ');
}

/**
 * @brief remove newlines from string
 * @param str string
 * @return none
 */
void remove_newlines(char *str)
{
	remove_chars(str, '\n');
}

/**
 * @brief remove quotations from string
 * @param str string
 * @return none
 */
void remove_quotations(char *str)
{
	remove_chars(str, '"');
}
