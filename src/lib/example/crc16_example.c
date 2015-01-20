#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedefs.h"
#include "crc16.h"

typedef struct _SKEY {
	BYTE data[12];
	BYTE random[2];
	BYTE crc[2];
} __attribute__((packed)) SKEY;

int main()
{
	SKEY skey;
	UINT16 crc;
	int random_num;

	memset((char *)&skey, 0, sizeof(SKEY));

	/* data */
	memcpy(skey.data, "12345678901", 12);
	/* random */
	random_num = random() % 65536;
	memcpy(skey.random, &random_num, sizeof(unsigned short));
	/* crc */
	crc = get_crc16((const char *)&skey, 14);
	printf("crc=%d,0x%X\n", crc, crc);
	crc = do_crc16(0xFFFF, (const char *)&skey, 14);
	printf("crc=%d,0x%X\n", crc, crc);
	memcpy(&skey.crc, &crc, sizeof(UINT16));

	printf("data=%s\n", (char *)&skey.data);
	printf("random=0x%X%X\n", skey.random[1], skey.random[0]);
	printf("crc=0x%X%X\n", skey.crc[1], skey.crc[0]);

	return 0;
}
