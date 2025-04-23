/* Copyright 2025, pan (pan_@disroot.org) */
/* SPDX-License-Identifier: MIT-0 */

#ifndef BLAYOUT_H
#define BLAYOUT_H

#include <stddef.h>  /* size_t, ptrdiff_t */
#include <stdint.h>  /* SIZE_MAX, uintptr_t */

struct blayout {
	size_t nmemb;
	size_t size;
	size_t align;
};

inline static size_t blcalc(size_t align,
                            ptrdiff_t offs,
                            size_t n,
                            const struct blayout *lays,
                            size_t prev_size)
{
	size_t i;
	size_t base = align + (size_t)offs;
	size_t pos = base;
	if (pos + prev_size < pos)
		return 0;

	pos += prev_size;
	for (i = 0; i < n; ++i) {
		size_t size;
		size_t pad;
		struct blayout l = lays[i];
		if (l.nmemb > SIZE_MAX / l.size)
			return 0;

		size = l.nmemb * l.size;
		pad = ~(pos - 1) & (l.align - 1);
		if (size + pad < size)
			return 0;

		size += pad;
		if (pos + size < pos)
			return 0;

		pos += size;
	}

	return pos - base;
}

inline static void *blnext(void *ptr, size_t curr_size, size_t next_align)
{
	ptr = (char *)ptr + curr_size;
	return (char *)ptr + (size_t)(~((uintptr_t)ptr - 1) & (next_align - 1));
}

inline static void *blprev(void *ptr, size_t prev_size, size_t prev_align)
{
	ptr = (char *)ptr - prev_size;
	return (char *)ptr - (size_t)((uintptr_t)ptr & (prev_align - 1));
}

inline static size_t blsizeof(const struct blayout *l)
{
	return l->nmemb * l->size;
}

#endif  /* BLAYOUT_H */
