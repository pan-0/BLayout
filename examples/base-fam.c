/* Copyright 2025, pan (pan_@disroot.org) */
/* SPDX-License-Identifier: MIT-0 */

/* See <https://old.reddit.com/r/C_Programming/comments/1age6gt/a_base_struct_inplace_of_a_flexible_array_member/> */

#include "blayout.h"
#include <assert.h>    /* static_assert */
#include <stdalign.h>  /* alignof */
#include <stddef.h>    /* size_t, NULL */
#include <stdio.h>     /* printf() */
#include <stdlib.h>    /* malloc(), free(), EXIT_FAILURE, EXIT_SUCCESS */

#define lengthof(A) (sizeof (A) / sizeof (A)[0])

struct Base {
    int x;
};

struct Derived {
    struct Base b;
    int y;
};

struct Wrapper {
    int z;
    /*struct Base b;*/
};

/* To be sure that `struct Wrapper` is allocated at the base. */
static_assert(alignof(struct Wrapper) <= BL_ALIGNMENT, "");

int main(void)
{
	const struct blayout l[] = {
		{1, sizeof(struct Wrapper), alignof(struct Wrapper)},
		{1, sizeof(struct Derived), alignof(struct Derived)}
	};
	size_t size = blcalc(BL_ALIGNMENT, 0, lengthof(l), l, 0);
	struct Wrapper *w = size == 0 ? NULL : malloc(size);
    if (w == NULL)
        return EXIT_FAILURE;

    /* Okay. */
    w->z = 1;

    /* Okay. */
	struct Base *b;
#	ifdef __clang__
		/* Clang fails to optimize `blnext()`, so we have to do a little bit more work. */
		static_assert(alignof(struct Wrapper) == alignof(struct Derived) &&
		              (sizeof(struct Wrapper) & (alignof(struct Wrapper) - 1)) == 0, "");
		b = (struct Base *b)(w + 1);
#	else
		b = blnext(w, blsizeof(&l[0]), l[1].align);
#	endif
    b->x = 2;

    /* Okay! */
    struct Derived *d = (struct Derived *)b;
    d->y = 3;

    printf("w->z=%d b->x=%d d->y=%d\n", w->z, b->x, d->y);

    d->b.x = 4;
    printf("w->z=%d b->x=%d d->y=%d\n", w->z, b->x, d->y);

    free(w);
    return EXIT_SUCCESS;
}
