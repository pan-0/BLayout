
/*
 * MIT No Attribution
 *
 * Copyright 2025 pan <pan_@disroot.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef BLAYOUT_H
#define BLAYOUT_H

/* size_t, ptrdiff_t, max_align_t [optional], offsetof() [<=C99] */
#include <stddef.h>
#include <stdint.h>  /* SIZE_MAX, uintptr_t */


/*
 * Boilerplate.
 */

#ifndef BL_ASSERT
#	include <assert.h>  /* assert() */
#	define BL_ASSERT assert
#endif

#ifndef BL_ALIGNMENT
#	if defined __cplusplus
#		include <cstddef>  /* std::max_align_t */
#		define BL_ALIGNMENT alignof(std::max_align_t)
#	elif !defined _MSC_VER && defined __STDC_VERSION__ && \
			__STDC_VERSION__ >= 201112L  /* C11 */
#		define BL_ALIGNMENT _Alignof(max_align_t)
#	else
		/*
		 * MSVC doesn't have `max_align_t`.
		 *
		 * <https://learn.microsoft.com/en-ie/answers/questions/1726147/why-max-align-t-not-defined-in-stddef-h-in-windows>
		 * <https://developercommunity.visualstudio.com/t/stdc11-should-add-max-align-t-to-stddefh/1386891>
		 * <https://github.com/emscripten-core/emscripten/pull/8908>
		 */
		union bl_max_align {
			long long ll;
			long double ld;
		};
#		if defined __STDC_VERSION__ && __STDC_VERSION__ < 201112L
#			ifdef __GNUC__
				/* <https://gcc.gnu.org/onlinedocs/gcc-2.95.3/gcc_4.html#SEC89> */
#				define BL_ALIGNMENT __alignof__(union bl_max_align)
#			else
				/* C99 and earlier standards don't support `_Alignof`. */
				struct bl_max_align99 {
					char c;
					union bl_max_align a;
				};
#				define BL_ALIGNMENT offsetof(struct bl_max_align99, a)
#			endif
#		else
#			define BL_ALIGNMENT _Alignof(union bl_max_align)
#		endif
#	endif
#endif

#ifndef BL_API
#	define BL_API static
#endif


/*
 * Types.
 */

typedef size_t blsize;  /* Can also be a `signed` type. */
#define BL_SIZE_MAX SIZE_MAX

struct blayout {
	blsize nmemb;
	blsize size;
	blsize align;
};


/*
 * Functions.
 */

#if defined __GNUC__
__attribute__((__always_inline__, __pure__)) inline
#elif defined _MSC_VER
__forceinline
#else
inline
#endif
BL_API blsize blcalc(blsize align,
                     ptrdiff_t offs,
                     blsize n,
                     const struct blayout *lays,
                     blsize prev_size)
{
	size_t base = (size_t)align + (size_t)offs;
	size_t pos = base;
	blsize i;

	BL_ASSERT(/* size >= 0 && prev_size >= 0 && */ n > 0 &&
	          lays != (void *)0 && align > 0 &&
	          ((size_t)align & ((size_t)align - 1)) == 0 && offs >= 0 &&
	          (size_t)align + (size_t)offs >= (size_t)align);

	if (pos + (size_t)prev_size < pos)
		return 0;

	pos += (size_t)prev_size;
	for (i = 0; i < n; ++i) {
		size_t size;
		size_t pad;
		struct blayout l = lays[i];
		BL_ASSERT(l.nmemb > 0 && l.size > 0 && l.align > 0 &&
		          ((size_t)l.align & ((size_t)l.align - 1)) == 0);
		if (l.nmemb > BL_SIZE_MAX / l.size)
			return 0;

		size = (size_t)l.nmemb * (size_t)l.size;
		pad = ~(pos - 1) & ((size_t)l.align - 1);
		if (size + pad < size)
			return 0;

		size += pad;
		if (pos + size < pos)
			return 0;

		pos += size;
	}

	return (blsize)(pos - base);
}

#ifdef __GNUC__
__attribute__((__returns_nonnull__, __nonnull__(1), __alloc_align__(3)))
#endif
inline
BL_API void *blnext(void *ptr, blsize curr_size, blsize next_align)
{
	size_t pad;
#	ifndef __clang__
		BL_ASSERT(ptr != (void *)0);
#	endif
	BL_ASSERT(/* curr_size >= 0 && */ next_align > 0 &&
	          ((size_t)next_align & ((size_t)next_align - 1)) == 0);
	ptr = (char *)ptr + curr_size;
	pad = (size_t)(~((uintptr_t)ptr - 1) & ((size_t)next_align - 1));
	return (char *)ptr + pad;
}

#ifdef __GNUC__
__attribute__((__returns_nonnull__, __nonnull__(1), __alloc_size__(2),
               __alloc_align__(3)))
#endif
inline
BL_API void *blprev(void *ptr, blsize prev_size, blsize prev_align)
{
#	ifndef __clang__
		BL_ASSERT(ptr != (void *)0);
#	endif
	BL_ASSERT(prev_size > 0 && prev_align > 0 &&
	          ((size_t)prev_align & ((size_t)prev_align - 1)) == 0);
	ptr = (char *)ptr - prev_size;
	ptr = (char *)ptr - (size_t)((uintptr_t)ptr & ((size_t)prev_align - 1));
	return ptr;
}

#ifdef __GNUC__
__attribute__((__always_inline__, __nonnull__(1), __pure__))
#endif
inline
BL_API blsize blsizeof(const struct blayout *l)
{
	blsize nmemb;
	blsize size;
#	ifndef __clang__
		BL_ASSERT(l != (void *)0);
#	endif
	nmemb = l->nmemb;
	size = l->size;
	BL_ASSERT(nmemb > 0 && size > 0 && l->align > 0 &&
	          ((size_t)l->align & ((size_t)l->align - 1)) == 0 &&
	          !(nmemb > BL_SIZE_MAX / size));
	return nmemb * size;
}

#endif  /* BLAYOUT_H */
