/* Copyright 2025, pan (pan_@disroot.org) */
/* SPDX-License-Identifier: MIT-0 */

/* BL_ALIGNMENT, struct blayout, blcalc(), blnext(), blsizeof() */
#include "blayout.h"
#include <assert.h>    /* assert() */
#include <stdalign.h>  /* alignof */
#include <stddef.h>    /* size_t, NULL */
#include <stdio.h>     /* printf() */
#include <stdlib.h>    /* malloc(), free() */

#define lengthof(A) (sizeof (A) / sizeof (A)[0])

int main(void)
{
	size_t i_len = 2;
	size_t d_len = 2;
	const struct blayout lays[] = {{i_len, sizeof(int),    alignof(int)   },
	                               {d_len, sizeof(double), alignof(double)}};
	size_t size = blcalc(BL_ALIGNMENT, 0, lengthof(lays), lays, 0);
	if (size == 0)
		return 1;

	void *block = malloc(size);
	if (block == NULL)
		return 1;

	int    *i = blnext(block,                  0, lays[0].align);
	double *d = blnext(    i, blsizeof(&lays[0]), lays[1].align);
	assert(i != NULL);
	assert(d != NULL);

	i[0] = 42;
	i[1] = 1337;

	d[0] = 2.71;
	d[1] = 3.14;

	printf("i[0]=%d i[1]=%d d[0]=%f d[1]=%f\n", i[0], i[1], d[0], d[1]);

	free(block);
	return 0;
}
