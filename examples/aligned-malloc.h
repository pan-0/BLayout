/* Copyright 2025, pan (pan_@disroot.org) */
/* SPDX-License-Identifier: MIT-0 */

/*
 * Equivalent to C11's `aligned_alloc()` but for C99. To free, you _must_ use
 * `aligned_free()`.
 *
 * Implemented as a "header library" for the sake of this example.
 */

#ifndef AM_H
#define AM_H

#include <stddef.h>  /* size_t */

#ifndef AM_API
#	define AM_API
#endif

#ifdef _MSC_VER
__declspec(restrict)
#endif
AM_API void *aligned_malloc(size_t alignment, size_t size)
#	ifdef __GNUC__
	__attribute__((__alloc_align__(1), __malloc__))
	/*
	 * This attribute is extremely buggy, atleast under GCC versions 11.0 -
	 * 14.2. See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96503>.
	 */
	/*__attribute__((__alloc_size__(2)))*/
#	endif
	;

AM_API void aligned_free(void *ptr);

#endif  /* AM_H */


/*
 * Implementation.
 */
#ifdef AM_IMPL

#include "blayout.h"  /* BL_ALIGNMENT, struct blayout, blcalc(), blprev() */
#include <errno.h>    /* errno, EINVAL, ENOMEM */
/*#include <stddef.h>   / * NULL, size_t, offsetof() */
#include <stdlib.h>   /* malloc(), free() */

#ifdef __GNUC__
#	define unlikely(x)  __builtin_expect(!!(x), 0)
#	define aligned(ptr) __builtin_assume_aligned((ptr), sizeof(void *))
#else
#	define unlikely(x)  (x)
#	define aligned(ptr) (ptr)
#endif

struct amhdr {
	void *blk;
};

struct amhdr_padded {
	char _c;
	struct amhdr hdr;
};

#define AMHDR_ALIGNMENT offsetof(struct amhdr_padded, hdr)

AM_API void *aligned_malloc(size_t alignment, size_t size)
{
	int err;
	if (unlikely(alignment == 0 || (alignment & (alignment - 1)) != 0
			|| alignment % sizeof(void *) != 0
			|| (size & (alignment - 1)) != 0)) {
		err = EINVAL;
		goto error;
	}

	if (size == 0)
		size = 1;  /* Alternatively: `return NULL;` */

	const struct blayout l[] = {
		{1, sizeof(struct amhdr), AMHDR_ALIGNMENT},
		{1, size, alignment}
	};
	size_t req = blcalc(BL_ALIGNMENT, 0, 2, l, 0);
	if (unlikely(req == 0)) {
		err = ENOMEM;
		goto error;
	}

	void *blk = malloc(req);
	if (unlikely(blk == NULL))
		goto error_malloc;

	void *end = (char *)blk + req;
	void *ptr = blprev(aligned(end), size, alignment);
	struct amhdr *hdr = blprev(aligned(ptr), sizeof *hdr, AMHDR_ALIGNMENT);
	hdr->blk = blk;
	return ptr;

error:
	errno = err;
error_malloc:
	return NULL;
}

AM_API void aligned_free(void *ptr)
{
	if (ptr != NULL) {
		struct amhdr *hdr = blprev(aligned(ptr), sizeof *hdr, AMHDR_ALIGNMENT);
		free(hdr->blk);
	}
}

#undef AMHDR_ALIGNMENT
#undef aligned
#undef unlikely

#undef AM_IMPL
#endif  /* AM_IMPL */
