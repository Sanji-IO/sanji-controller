#include <stdio.h>
#include "bswap.h"

int main()
{
	unsigned short num_16 = 0x00ff;
	unsigned int num_32 = 0x00ff00ff;

	printf("%X -> %X\n", num_16, BSWAP16(num_16));
	printf("%X -> %X\n", num_32, BSWAP32(num_32));

	return 0;
}
