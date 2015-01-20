#include <stdio.h>
#include <unistd.h>
#include "random_util.h"

int main()
{
	int i;

	for (i = 0; i < 10; i++) {
		printf("%d\n", generate_random(RAND_MODE_SEQ));
	}

	for (i = 0; i < 10; i++) {
		printf("%d\n", generate_random(RAND_MODE_RANDOM));
		sleep(1);
	}

	return 0;
}
