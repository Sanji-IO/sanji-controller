#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strext.h"

#define STR_SIZE 16

int main()
{
	/* test strtrim(), strrtrim(), strltrim() */
	char str1[] = "012345678911111";
	char str2[STR_SIZE] = "012345678911111";
	char *str3;

	str3 = (char *)calloc(STR_SIZE, sizeof(char));
	sprintf(str3, "012345678911111");
	printf("%lu\n", sizeof(str1));
	printf("%lu\n", sizeof(str2));
	printf("%lu\n", sizeof(str3));

	str3 = strrtrim(str3, "19");
	printf("%s\n", str3);
	str3 = strrtrim(str3, "45");
	printf("%s\n", str3);

	str3 = strltrim(str3, "210");
	printf("%s\n", str3);
	str3 = strltrim(str3, "98");
	printf("%s\n", str3);

	memset(str3, 0, STR_SIZE);
	sprintf(str3, "012345678911111");
	str3 = strtrim(str3, "0129");
	printf("%s\n", str3);

	strtrim(str1, "0129");
	printf("%s\n", str1);
	strtrim(str2, "0129");
	printf("%s\n", str2);

	free(str3);

	/* test strlwr(), strupr() */
	char *str4;
	char *str5;
	str4 = (char *)calloc(STR_SIZE, sizeof(char));
	sprintf(str4, "aBcDeFgHiJ12");
	str5 = (char *)calloc(STR_SIZE, sizeof(char));
	sprintf(str5, "aBcDeFgHiJ12");

	str4 = strlwr(str4);
	printf("%s\n", str4);
	str5 = strupr(str5);
	printf("%s\n", str5);

	free(str4);
	free(str5);

	return 0;
}
