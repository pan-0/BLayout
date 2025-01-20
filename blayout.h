
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
 * Uncomment any of these if you want to override the default values.
 */

/*#define BL_SIZEMAX   SIZE_MAX*/
/*#define BL_ALIGNMENT alignof(max_align_t)*/
/*#define BL_API       static*/
/*#define BL_ASSERT    assert*/
/*#define BL_INLINE    inline*/
/*#define BL_DEBUG     0*/


/*
 * Boilerplate.
 */

#if !defined BL_DEBUG || BL_DEBUG == 0
#define BL_ATTR(attr) __attribute__((attr))
#if 0 && !defined BL_ASSERT
#define BL_ASSERT(_)
#endif
#elif BL_DEBUG == 1
#define BL_ATTR(attr) __attribute__((attr))
#ifndef BL_ASSERT
#include <assert.h>  /* assert() */
#define BL_ASSERT assert
#endif
#elif BL_DEBUG == 2
#define BL_ATTR(_)
#ifndef BL_ASSERT
#include <assert.h>  /* assert() */
#define BL_ASSERT assert
#endif
#else
#error "invalid `BL_DEBUG` value, must be `0`, `1`, or `2`"
#endif

#ifndef BL_ALIGNMENT
/* C++ */
#if defined __cplusplus
#if __cplusplus >= 201103L  /* C++11 */
#include <cstddef>  /* std::max_align_t */
#define BL_ALIGNMENT alignof(std::max_align_t)
/* else :shrug-face: */
#endif
/* !MSVC && >=C11 */
#elif !defined _MSC_VER && defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L
#if __STDC_VERSION__ >= 202311L  /* C23 */
#define BL_ALIGNMENT alignof(max_align_t)
#else
#define BL_ALIGNMENT _Alignof(max_align_t)
#endif
/*
 * MSVC doesn't have `max_align_t`.
 *
 * <https://learn.microsoft.com/en-ie/answers/questions/1726147/why-max-align-t-not-defined-in-stddef-h-in-windows>
 * <https://developercommunity.visualstudio.com/t/stdc11-should-add-max-align-t-to-stddefh/1386891>
 * <https://github.com/emscripten-core/emscripten/pull/8908>
 */
#else
union bl_max_align {
	long long _ll;
	long double _ld;
};
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 202311L  /* C23 */
#define BL_ALIGNMENT alignof(union bl_max_align)
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L  /* C11 */
#define BL_ALIGNMENT _Alignof(union bl_max_align)
#elif defined _MSC_VER
/* <https://learn.microsoft.com/en-us/cpp/cpp/alignment-cpp-declarations?view=msvc-170> */
#define BL_ALIGNMENT __alignof(union bl_max_align)
#elif defined __GNUC__
/* <https://gcc.gnu.org/onlinedocs/gcc-2.95.3/gcc_4.html#SEC89> */
#define BL_ALIGNMENT __alignof__(union bl_max_align)
#else
/* C99 and earlier standards don't support `_Alignof`. */
struct bl_max_align99 {
	char _c;
	union bl_max_align _a;
};
#define BL_ALIGNMENT offsetof(struct bl_max_align99, _a)
#endif
#endif
#endif

#ifndef BL_API
#define BL_API static
#endif

#ifndef BL_INLINE
#define BL_INLINE inline
#define BL_INLINE_USER 0
#else
#define BL_INLINE_USER 1
#endif


/*
 * Types.
 */

typedef size_t blsize;       /* Can also be a `signed` type. */
#ifndef BL_SIZEMAX
#define BL_SIZEMAX SIZE_MAX  /* Must be `> 0`. */
#endif

struct blayout {
	blsize nmemb;
	blsize size;
	blsize align;
};


/*
 * Functions.
 */

#if defined __GNUC__
__attribute__((__pure__))
#if !BL_INLINE_USER
BL_ATTR(__always_inline__)
#endif
BL_INLINE
#elif !BL_INLINE_USER && defined _MSC_VER
__forceinline
#else
BL_INLINE
#endif
BL_API blsize blcalc(blsize _align,
                     ptrdiff_t _offs,
                     blsize _n,
                     const struct blayout *_lays,
                     blsize _prev_size)
{
	size_t _base = (size_t)_align + (size_t)_offs;
	size_t _pos = _base;
	blsize _i;

#if defined BL_DEBUG && BL_DEBUG >= 1
	BL_ASSERT(_align > 0 && ((size_t)_align & ((size_t)_align - 1)) == 0 &&
	          _offs >= 0 && _n > 0 && _lays != (void *)0 &&
	          /* _prev_size >= 0 && */ _base >= (size_t)_align);
#endif

	if (_pos + (size_t)_prev_size < _pos)
		return 0;

	_pos += (size_t)_prev_size;
	for (_i = 0; _i < _n; ++_i) {
		size_t _size;
		size_t _pad;
		struct blayout _l = _lays[_i];
#if defined BL_DEBUG && BL_DEBUG >= 1
		BL_ASSERT(_l.nmemb > 0 && _l.size > 0 && _l.align > 0 &&
		          ((size_t)_l.align & ((size_t)_l.align - 1)) == 0);
#endif
		if (_l.nmemb > BL_SIZEMAX / _l.size)
			return 0;

		_size = (size_t)_l.nmemb * (size_t)_l.size;
		_pad = ~(_pos - 1) & ((size_t)_l.align - 1);
		if (_size + _pad < _size)
			return 0;

		_size += _pad;
		if (_pos + _size < _pos)
			return 0;

		_pos += _size;
	}

	_pos -= _base;
	return _pos > BL_SIZEMAX ? 0 : (blsize)_pos;
}

#ifdef __GNUC__
BL_ATTR(__returns_nonnull__) BL_ATTR(__nonnull__(1))
__attribute__((__alloc_align__(3)))
#endif
BL_INLINE
BL_API void *blnext(void *_ptr, blsize _curr_size, blsize _next_align)
{
	size_t _pad;
#ifdef BL_DEBUG
#if BL_DEBUG >= 2
	BL_ASSERT(_ptr != (void *)0);
#endif
#if BL_DEBUG >= 1
	BL_ASSERT(/* _curr_size >= 0 && */ _next_align > 0 &&
	          ((size_t)_next_align & ((size_t)_next_align - 1)) == 0);
#endif
#endif
	_ptr = (char *)_ptr + _curr_size;
	_pad = (size_t)(~((uintptr_t)_ptr - 1) & ((size_t)_next_align - 1));
	return (char *)_ptr + _pad;
}

#ifdef __GNUC__
BL_ATTR(__returns_nonnull__) BL_ATTR(__nonnull__(1))
__attribute__((__alloc_size__(2), __alloc_align__(3)))
#endif
BL_INLINE
BL_API void *blprev(void *_ptr, blsize _prev_size, blsize _prev_align)
{
	size_t _pad;
#ifdef BL_DEBUG
#if BL_DEBUG >= 2
	BL_ASSERT(_ptr != (void *)0);
#endif
#if BL_DEBUG >= 1
	BL_ASSERT(_prev_size > 0 && _prev_align > 0 &&
	          ((size_t)_prev_align & ((size_t)_prev_align - 1)) == 0);
#endif
#endif
	_ptr = (char *)_ptr - _prev_size;
	_pad = (size_t)((uintptr_t)_ptr & ((size_t)_prev_align - 1));
	return (char *)_ptr - _pad;
}

#ifdef __GNUC__
BL_ATTR(__nonnull__(1)) __attribute__((__pure__))
#if !BL_INLINE_USER
BL_ATTR(__always_inline__)
#endif
#endif
BL_INLINE
BL_API blsize blsizeof(const struct blayout *_l)
{
	blsize _nmemb;
	blsize _size;
#if defined BL_DEBUG && BL_DEBUG >= 2
	BL_ASSERT(_l != (void *)0);
#endif
	_nmemb = _l->nmemb;
	_size = _l->size;
#if defined BL_DEBUG && BL_DEBUG >= 1
	BL_ASSERT(_nmemb > 0 && _size > 0 && _l->align > 0 &&
	          ((size_t)_l->align & ((size_t)_l->align - 1)) == 0 &&
	          !(_nmemb > BL_SIZEMAX / _size));
#endif
	return _nmemb * _size;
}

#if !BL_INLINE_USER
#undef BL_INLINE
#endif
#undef BL_INLINE_USER
#undef BL_ATTR

#endif  /* BLAYOUT_H */
