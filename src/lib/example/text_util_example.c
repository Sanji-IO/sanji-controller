#include <stdio.h>
#include "text_util.h"

int main()
{
	unsigned char bin_string[] = "123456789";
	unsigned char bin_string_r[32];
	char hex_string[32];

	bin2hex(bin_string, 9, hex_string);
	printf("%s\n", hex_string);

	hex2bin(hex_string, 18, bin_string_r);
	printf("%s\n", bin_string_r);

	char trailing_string[] = "hello   ";
	printf("'%s'\n", trailing_string);
	remove_trailing_spaces(trailing_string);
	printf("'%s'\n", trailing_string);

	/* remove character */
	char new_string[] = " hello \"simon\".   \nmay I help you?  ";
	printf("'%s'\n", new_string);

	remove_chars(new_string, 'h');
	printf("'%s'\n", new_string);

	remove_spaces(new_string);
	printf("'%s'\n", new_string);

	remove_quotations(new_string);
	printf("'%s'\n", new_string);

	remove_newlines(new_string);
	printf("'%s'\n", new_string);

	return 0;
}
