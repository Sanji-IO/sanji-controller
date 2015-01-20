/**
 * @file random_util.c
 * @brief Definition of random utility
 * @date 2014-09-04
 * @version 1.0.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "random_util.h"

/**
 * @brief gererate a random number between 0 and RAND_MAX (same as INT_MAX)
 * @param mode sequentail number or random number
 * @return random number
 * @remarks return value between 0 and RAND_MAX
 */
int generate_random(int mode)
{
#ifndef WIN32
	static int num = 0;
	unsigned int seed;
	FILE *urandom = NULL;

	switch (mode) {
	case RAND_MODE_SEQ:
		if (num++ == RAND_MAX) num = 0;
		return num;
		break;
	case RAND_MODE_RANDOM:
	default:
		urandom = fopen("/dev/urandom", "r");
		if (urandom) {
			fread(&seed, sizeof(int), 1, urandom);
			fclose(urandom);
		} else {
			seed = time(NULL);
		}
		srand(seed);
		return rand();
	}
#else
#endif
}
