
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

/* NULL, size_t, ptrdiff_t, max_align_t [optional], offsetof() [<=C99] */
#include <stddef.h>
#include <stdint.h>  /* SIZE_MAX, uintptr_t */


/*
 * Uncomment any of these if you want to override the default values.
 */

/*#define BL_SIZEMAX   SIZE_MAX*/
/*#define BL_ALIGNMENT alignof(max_align_t)*/
/*#define BL_ASSERT    assert*/
/*#define BL_INLINE    inline*/
/*#define BL_DEBUG     0*/
/*#define BL_CONST     0*/


/*
 * Types.
 */

typedef size_t blsize;       /* Can also be a `signed` type. */
#ifndef BL_SIZEMAX
#define BL_SIZEMAX SIZE_MAX  /* Must be in (0, SIZE_MAX]. */
#endif

typedef uintptr_t bluptr;

struct blayout {
	blsize nmemb;
	blsize size;
	blsize align;
};


/*
 * Boilerplate.
 */

#define BLALIGNED(size, align) \
	(((size) | 0ul) + (~(((size) | 0ul) - 1) & (((align) | 0ul) - 1)))

#if defined BL_CONST && !(BL_CONST >= 0 && BL_CONST <= 3)
#error "invalid `BL_CONST` value, must be `0`, `1`, `2` or `3`"
#elif defined __cplusplus && __cplusplus >= 202302L /* C++23 */ \
		&& (!defined BL_CONST || BL_CONST <= 1)
#warn "recommend `BL_CONST` with a value of `2`"
#endif

#if !defined BL_DEBUG || BL_DEBUG == 0
#define BL_PRIV_ATTR(attr) __attribute__((attr))
#if 0 && !defined BL_ASSERT
#define BL_ASSERT(_)
#endif
#elif BL_DEBUG == 1
#define BL_PRIV_ATTR(attr) __attribute__((attr))
#ifndef BL_ASSERT
#include <assert.h>  /* assert() */
#define BL_ASSERT assert
#endif
#elif BL_DEBUG == 2
#define BL_PRIV_ATTR(_)
#ifndef BL_ASSERT
#include <assert.h>  /* assert() */
#define BL_ASSERT assert
#endif
#elif BL_DEBUG == 3
#define BL_PRIV_ATTR(_)
#ifndef BL_ASSERT
#include <assert.h>  /* assert() */
#define BL_ASSERT assert
#endif
#if defined __cplusplus
/* Can't do it. */
#define BL_PRIV_IASSERT(x, _x, msg) BL_ASSERT((_x) && #x msg)
#elif defined __GNUC__
#define BL_PRIV_STR_(X) #X
#define BL_PRIV_STR(X)  BL_PRIV_STR_(X)
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 202311L  /* C23 */
#define BL_PRIV_IASSERT(x, _x, msg)                           \
    static_assert(                                            \
        __builtin_choose_expr(__builtin_constant_p(x), x, 1), \
        __FILE__":"BL_PRIV_STR(__LINE__)": Static assertion `"#x"` failed; " msg); \
    if (!__builtin_constant_p(x))                             \
        BL_ASSERT((_x) && #x msg)
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L  /* C11 */
#define BL_PRIV_IASSERT(x, _x, msg)                           \
    _Static_assert(                                           \
        __builtin_choose_expr(__builtin_constant_p(x), x, 1), \
        __FILE__":"BL_PRIV_STR(__LINE__)": Static assertion `"#x"` failed; " msg); \
    if (!__builtin_constant_p(x))                             \
        BL_ASSERT((_x) && #x msg)
#else
#undef BL_PRIV_STR
#undef BL_PRIV_STR_
#define BL_PRIV_IASSERT(x, _x, msg)                         \
    do {                                                    \
        struct bl_priv_iassert {                            \
            unsigned : __builtin_constant_p(x) ? -!(x) : 0; \
        };                                                  \
        if (!__builtin_constant_p(x))                       \
            BL_ASSERT((_x) && #x msg);                      \
    } while (0)
#endif
#endif
#else
#error "invalid `BL_DEBUG` value, must be `0`, `1`, `2` or `3`"
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
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 202311L  /* C23 */
struct bl_priv_max_align {
	alignas(long long) long long _bl_priv_ll;
	alignas(long double) long double _bl_priv_ld;
};
#define BL_ALIGNMENT alignof(struct bl_priv_max_align)
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L  /* C11 */
struct bl_priv_max_align {
	_Alignas(long long) long long _bl_priv_ll;
	_Alignas(long double) long double _bl_priv_ld;
};
#define BL_ALIGNMENT _Alignof(struct bl_priv_max_align)
#elif defined _MSC_VER  /* MSVC */
struct bl_priv_max_align {
	long long _bl_priv_ll;
	long double _bl_priv_ld;
};
/* <https://learn.microsoft.com/en-us/cpp/cpp/alignment-cpp-declarations?view=msvc-170> */
#define BL_ALIGNMENT __alignof(struct bl_priv_max_align)
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L  /* C99 */
#ifdef __GNUC__
/* <https://gcc.gnu.org/onlinedocs/gcc-2.95.3/gcc_4.html#SEC89> */
struct bl_priv_max_align {
	long long _bl_priv_ll
		__attribute__((__aligned__(__alignof__(long long))));
	long double _bl_priv_ld
		__attribute__((__aligned__(__alignof__(long double))));
};
#define BL_ALIGNMENT __alignof__(struct bl_priv_max_align)
#else
/* C99 and earlier standards don't support `_Alignof`. */
struct bl_priv_max_align {
	char _bl_priv_c;
	struct {
		long long _bl_priv_ll;
		long double _bl_priv_ld;
	} _bl_priv_a;
};
#define BL_ALIGNMENT offsetof(struct bl_priv_max_align, _bl_priv_a)
#endif
#endif
#endif

#define BL_API static

#ifndef BL_INLINE
#define BL_INLINE inline
#define BL_PRIV_INLINE_USER 0
#else
#define BL_PRIV_INLINE_USER 1
#endif

#ifdef __GNUC__
#define BL_PRIV_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define BL_PRIV_UNLIKELY(x) (x)
#endif

#if (!defined BL_DEBUG || BL_DEBUG == 0) && !BL_PRIV_INLINE_USER
#if defined __GNUC__
#define BL_PRIV_INLINE_ALWAYS BL_PRIV_ATTR(__always_inline__) BL_INLINE
#elif defined _MSC_VER
#define BL_PRIV_INLINE_ALWAYS __forceinline
#endif
#endif
#ifndef BL_PRIV_INLINE_ALWAYS
#define BL_PRIV_INLINE_ALWAYS BL_INLINE
#endif


/*
 * Functions.
 */

#if defined __GNUC__
__attribute__((__pure__))  /* <- always use _some_ attributes. */
#endif
BL_PRIV_INLINE_ALWAYS
BL_API blsize bl_priv_calc(register const blsize _align,
                           register const ptrdiff_t _offs,
                           register const blsize _n,
                           register const struct blayout *const _lays,
                           register const blsize _prev_size)
{
	register const size_t _base = (size_t)_align + (size_t)_offs;
	register size_t _pos = _base;

#if defined BL_DEBUG && BL_DEBUG >= 1 && !defined BL_PRIV_IASSERT
	BL_ASSERT(_align > 0 && "`align` must be a power of 2");
	BL_ASSERT(((size_t)_align & ((size_t)_align - 1)) == 0
	          && "`align` must be a power of 2");
	BL_ASSERT(_offs >= 0 && "`offs` must be non-negative");
	BL_ASSERT(_n > 0 && _n <= SIZE_MAX && "`n` must be in (0, SIZE_MAX]");
	BL_ASSERT(_lays != NULL && "`lays` must point to a non-zero-sized array");
	/*BL_ASSERT(_prev_size >= 0); */
	BL_ASSERT(_base >= (size_t)_align &&
	          "detected wrap-around; too large `align` and/or `offs`");
#endif

	if (BL_PRIV_UNLIKELY(_pos + (size_t)_prev_size < _pos))
		return 0;

	_pos += (size_t)_prev_size;
	{
		register blsize _i;
		for (_i = 0; _i < _n; ++_i) {
			const struct blayout _l = _lays[_i];
#if defined BL_DEBUG && BL_DEBUG >= 1
			BL_ASSERT(_l.nmemb > 0 && _l.nmemb <= SIZE_MAX
					  && "layout `.nmemb` must be in (0, SIZE_MAX]");
			BL_ASSERT(_l.size > 0 && _l.size <= SIZE_MAX
					  && "layout `.size` must be in (0, SIZE_MAX]");
			BL_ASSERT(_l.align > 0 && "layout alignment must be a power of 2");
			BL_ASSERT(((size_t)_l.align & ((size_t)_l.align - 1)) == 0
					  && "layout alignment must be a power of 2");
#endif
			/* TODO: This actually generates a `div` under x64 MSVC... */
			if (BL_PRIV_UNLIKELY(_l.nmemb > BL_SIZEMAX / _l.size))
				return 0;

			{
				register size_t _size = (size_t)_l.nmemb * (size_t)_l.size;
				register const size_t _pad =
					~(_pos - 1) & ((size_t)_l.align - 1);
				if (BL_PRIV_UNLIKELY(_size + _pad < _size))
					return 0;

				_size += _pad;
				if (BL_PRIV_UNLIKELY(_pos + _size < _pos))
					return 0;

				_pos += _size;
			}
		}
	}

	_pos -= _base;
	return BL_PRIV_UNLIKELY(_pos > BL_SIZEMAX) ? 0 : (blsize)_pos;
}

#undef BL_PRIV_UNLIKELY

#ifdef __GNUC__
BL_PRIV_ATTR(__returns_nonnull__) BL_PRIV_ATTR(__nonnull__(1))
__attribute__((__alloc_align__(3)))
#endif
BL_PRIV_INLINE_ALWAYS
BL_API void *bl_priv_next(register void *_ptr,
                          register const blsize _curr_size,
                          register const blsize _next_align)
{
#if defined BL_DEBUG && !defined BL_PRIV_IASSERT
#if BL_DEBUG >= 2
	BL_ASSERT(_ptr != NULL && "`ptr` cannot be NULL");
#endif
#if BL_DEBUG >= 1
	/*BL_ASSERT(_curr_size >= 0);*/
	BL_ASSERT(_next_align > 0 && "`next_align` must be a power of 2");
	BL_ASSERT(((size_t)_next_align & ((size_t)_next_align - 1)) == 0
	          && "`next_align` must be a power of 2");
#endif
#endif
	_ptr = (char *)_ptr + _curr_size;
	{
		register const size_t _pad = (size_t)(~((bluptr)_ptr - 1)
		                                      & ((size_t)_next_align - 1));
		return (char *)_ptr + _pad;
	}
}

#ifdef __GNUC__
BL_PRIV_ATTR(__returns_nonnull__) BL_PRIV_ATTR(__nonnull__(1))
__attribute__((__alloc_align__(3)))
/*
 * This attribute is extremely buggy, atleast under GCC versions 11.0 -
 * 14.2. See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=96503>.
 */
/*__attribute__((__alloc_size__(2)))*/
#endif
BL_PRIV_INLINE_ALWAYS
BL_API void *bl_priv_prev(register void *_ptr,
                          register const blsize _prev_size,
                          register const blsize _prev_align)
{
#if defined BL_DEBUG && !defined BL_PRIV_IASSERT
#if BL_DEBUG >= 2
	BL_ASSERT(_ptr != NULL && "`ptr` can't be NULL");
#endif
#if BL_DEBUG >= 1
	BL_ASSERT(_prev_size > 0 && _prev_size <= SIZE_MAX
	          && "`prev_size` must be in (0, SIZE_MAX]");
	BL_ASSERT(_prev_align > 0 && "`prev_align` must be a power of 2");
	BL_ASSERT(((size_t)_prev_align & ((size_t)_prev_align - 1)) == 0
	          && "`prev_align` must be a power of 2");
#endif
#endif
	_ptr = (char *)_ptr - _prev_size;
	{
		register const size_t _pad = (size_t)((bluptr)_ptr
		                                      & ((size_t)_prev_align - 1));
		return (char *)_ptr - _pad;
	}
}

#ifdef __GNUC__
/* This can be lifted in the future, once `NULL + 0` becomes defined. */
BL_PRIV_ATTR(__returns_nonnull__) BL_PRIV_ATTR(__nonnull__(1))
#endif
BL_PRIV_INLINE_ALWAYS
BL_API void *bl_priv_memb(register void *const _obj,
                          register const blsize _size,
                          register const ptrdiff_t _idx)
{
#if defined BL_DEBUG && !defined BL_PRIV_IASSERT
#if BL_DEBUG >= 2
	BL_ASSERT(_obj != NULL && "`obj` can't be NULL");
#endif
#if BL_DEBUG >= 1
	BL_ASSERT(_size > 0 && "`size` must be in (0, SIZE_MAX]");
#endif
#endif
	return (char *)_obj + (ptrdiff_t)_size * _idx;
}

#if defined BL_CONST && BL_CONST >= 1

#if BL_CONST >= 2
#if defined __cplusplus
#define BL_PRIV_UNCONST(ptr) const_cast<void *>(static_cast<const void *>(ptr))
#elif BL_CONST == 3
/*
 * Do this rigmarole to workaround `-Wcast-qual`. GCC/Clang silently accept
 * this. And it's fine anyway, since the underlying implementations already
 * depend on `uintptr_t`.
 */
#define BL_PRIV_UNCONST(ptr) ((void *)(bluptr)(const void *)(ptr))
#else
#define BL_PRIV_UNCONST(ptr) ((void *)(ptr))
#endif
#endif

#ifdef __GNUC__
BL_PRIV_ATTR(__returns_nonnull__) BL_PRIV_ATTR(__nonnull__(1))
__attribute__((__alloc_align__(3)))
#endif
BL_PRIV_INLINE_ALWAYS
BL_API const void *bl_priv_nextc(register const void *_ptr,
                                 register const blsize _curr_size,
                                 register const blsize _next_align)
{
#if defined BL_DEBUG && !defined BL_PRIV_IASSERT
#if BL_DEBUG >= 2
	BL_ASSERT(_ptr != NULL && "`ptr` cannot be NULL");
#endif
#if BL_DEBUG >= 1
	/*BL_ASSERT(_curr_size >= 0);*/
	BL_ASSERT(_next_align > 0 && "`next_align` must be a power of 2");
	BL_ASSERT(((size_t)_next_align & ((size_t)_next_align - 1)) == 0
	          && "`next_align` must be a power of 2");
#endif
#endif
	_ptr = (const char *)_ptr + _curr_size;
	{
		register const size_t _pad = (size_t)(~((bluptr)_ptr - 1)
		                                      & ((size_t)_next_align - 1));
		return (const char *)_ptr + _pad;
	}
}

#ifdef __GNUC__
BL_PRIV_ATTR(__returns_nonnull__) BL_PRIV_ATTR(__nonnull__(1))
__attribute__((__alloc_align__(3)))
/*__attribute__((__alloc_size__(2)))*/
#endif
BL_PRIV_INLINE_ALWAYS
BL_API const void *bl_priv_prevc(register const void *_ptr,
                                 register const blsize _prev_size,
                                 register const blsize _prev_align)
{
#if defined BL_DEBUG && !defined BL_PRIV_IASSERT
#if BL_DEBUG >= 2
	BL_ASSERT(_ptr != NULL && "`ptr` can't be NULL");
#endif
#if BL_DEBUG >= 1
	BL_ASSERT(_prev_size > 0 && _prev_size <= SIZE_MAX
	          && "`prev_size` must be in (0, SIZE_MAX]");
	BL_ASSERT(_prev_align > 0 && "`prev_align` must be a power of 2");
	BL_ASSERT(((size_t)_prev_align & ((size_t)_prev_align - 1)) == 0
	          && "`prev_align` must be a power of 2");
#endif
#endif
	_ptr = (const char *)_ptr - _prev_size;
	{
		register const size_t _pad = (size_t)((bluptr)_ptr
		                                      & ((size_t)_prev_align - 1));
		return (const char *)_ptr - _pad;
	}
}

#ifdef __GNUC__
BL_PRIV_ATTR(__returns_nonnull__) BL_PRIV_ATTR(__nonnull__(1))
#endif
BL_PRIV_INLINE_ALWAYS
BL_API const void *bl_priv_membc(register const void *const _obj,
                                 register const blsize _size,
                                 register const ptrdiff_t _idx)
{
#if defined BL_DEBUG && !defined BL_PRIV_IASSERT
#if BL_DEBUG >= 2
	BL_ASSERT(_obj != NULL && "`obj` can't be NULL");
#endif
#if BL_DEBUG >= 1
	BL_ASSERT(_size > 0 && "`size` must be in (0, SIZE_MAX]");
#endif
#endif
	return (const char *)_obj + (ptrdiff_t)_size * _idx;
}

#endif  /* `const`-qualified variants. */

#if !defined BL_PRIV_IASSERT

#define blcalc(align, offs, n, lays, prev_size) \
	bl_priv_calc(align, offs, n, lays, prev_size)
#define blnextc(ptr, curr_size, next_align) \
	bl_priv_nextc(ptr, curr_size, next_align)
#define blprevc(ptr, prev_size, prev_align) \
	bl_priv_nextc(ptr, prev_size, prev_align)
#define blmembc(obj, size, idx) bl_priv_membc(obj, size, idx)

#if defined __GNUC__ && (!defined BL_DEBUG || BL_DEBUG == 0)
__attribute__((__const__))
#endif
BL_PRIV_INLINE_ALWAYS
BL_API blsize blaligned(register const blsize _size,
                        register const blsize _align)
{
#if defined BL_DEBUG && BL_DEBUG >= 1
	BL_ASSERT(_size > 0 && _size <= SIZE_MAX
	          && "`size` must be in (0, SIZE_MAX]");
	BL_ASSERT(_align > 0 && "`align` must be a power of 2");
	BL_ASSERT(((size_t)_align & ((size_t)_align - 1)) == 0
	          && "`align` must be a power of 2");
#endif
	register const size_t _r =
		(size_t) BLALIGNED((size_t)_size, (size_t)_align);
#if defined BL_DEBUG && BL_DEBUG >= 1
	BL_ASSERT(_r >= (size_t)_size && "detected wrap-around in `blaligned()`");
	BL_ASSERT(_r <= (size_t)BL_SIZEMAX
	          && "`blaligned()` result can't fit in `blsize`");
#endif
	return (blsize)_r;
}

#ifdef __GNUC__
BL_PRIV_ATTR(__nonnull__(1)) __attribute__((__pure__))
#endif
BL_PRIV_INLINE_ALWAYS
BL_API blsize blsizeof(register const struct blayout *const _l)
{
#if defined BL_DEBUG && BL_DEBUG >= 2
	BL_ASSERT(_l != NULL && "`l` cannot be NULL");
#endif
	{
		register const blsize _nmemb = _l->nmemb;
#if defined BL_DEBUG && BL_DEBUG >= 1
		BL_ASSERT(_nmemb > 0 && _nmemb <= SIZE_MAX
				  && "layout `.nmemb` must be in (0, SIZE_MAX]");
#endif
		{
			register const blsize _size = _l->size;
#if defined BL_DEBUG && BL_DEBUG >= 1
			BL_ASSERT(_size > 0 && _size <= SIZE_MAX
					  && "layout `.size` must be in (0, SIZE_MAX]");
			BL_ASSERT(!(_nmemb > BL_SIZEMAX / _size)
					  && "object layout is too large");
			BL_ASSERT(_l->align > 0
			          && "layout alignment must be a power of 2");
			BL_ASSERT(((size_t)_l->align & ((size_t)_l->align - 1)) == 0
					  && "layout alignment must be a power of 2");
#endif
			return _nmemb * _size;
		}
	}
}

#if !defined BL_CONST || BL_CONST <= 1
#define blnext(ptr, curr_size, next_align) \
	bl_priv_next(ptr, curr_size, next_align)
#define blprev(ptr, prev_size, prev_align) \
	bl_priv_prev(ptr, prev_size, prev_align)
#define blmemb(obj, size, idx) bl_priv_memb(obj, size, idx)
#elif defined BL_CONST && BL_CONST >= 2
/*
 * (Ab)use the conditional operator. See the examples included in the C
 * standard to understand how this works.
 */
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L  /* C11 */
#define blnext(ptr, curr_size, next_align)                            \
    _Generic(1 ? (ptr) : BL_PRIV_UNCONST(ptr),                        \
             void *:       bl_priv_next,                              \
             const void *: bl_priv_nextc)(ptr, curr_size, next_align)
#define blprev(ptr, prev_size, prev_align)                            \
    _Generic(1 ? (ptr) : BL_PRIV_UNCONST(ptr),                        \
             void *:       bl_priv_prev,                              \
             const void *: bl_priv_prevc)(ptr, prev_size, prev_align)
#define blmemb(obj, size, idx)                            \
    _Generic(1 ? (obj) : BL_PRIV_UNCONST(obj),            \
             void *:       bl_priv_memb,                  \
             const void *: bl_priv_membc)(obj, size, idx)
#else
#define blnext(ptr, curr_size, next_align) \
	(1 ? bl_priv_next(BL_PRIV_UNCONST(ptr), curr_size, next_align) : (ptr))
#define blprev(ptr, prev_size, prev_align) \
	(1 ? bl_priv_prev(BL_PRIV_UNCONST(ptr), prev_size, prev_align) : (ptr))
#define blmemb(obj, size, idx) \
	(1 ? bl_priv_memb(BL_PRIV_UNCONST(obj), size, idx) : (obj))
#endif
#endif

#else

#if defined __cplusplus && __cplusplus >= 201103L  /* C++11 */
/*
 * Use "Lvalue closures" instead of statement expressions under C++11.
 *
 * See <https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2635.pdf>.
 */
#define BL_PRIV_STMT_EXPR_BEGIN     ([&](void){
#define BL_PRIV_STMT_EXPR_RET       return
#define BL_PRIV_STMT_EXPR_END       }())
#define BL_PRIV_STMT_EXPR_BEGIN_SUB
#define BL_PRIV_STMT_EXPR_RET_SUB
#define BL_PRIV_STMT_EXPR_END_SUB
#else
#define BL_PRIV_STMT_EXPR_BEGIN (__extension__ ({
#define BL_PRIV_STMT_EXPR_RET
#define BL_PRIV_STMT_EXPR_END   }))
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L  /* C99 */
#define BL_PRIV_STMT_EXPR_BEGIN_SUB
#define BL_PRIV_STMT_EXPR_RET_SUB
#define BL_PRIV_STMT_EXPR_END_SUB
#else
#define BL_PRIV_STMT_EXPR_BEGIN_SUB (__extension__ ({
#define BL_PRIV_STMT_EXPR_RET_SUB
#define BL_PRIV_STMT_EXPR_END_SUB   }));
#endif
#endif

#define blcalc(align, offs, n, lays, prev_size)                       \
    BL_PRIV_STMT_EXPR_BEGIN                                           \
    register const blsize _bl_priv_align = (align);                   \
    BL_PRIV_IASSERT((align) > 0, _bl_priv_align > 0,                  \
                    "`align` must be a power of 2");                  \
    BL_PRIV_IASSERT(                                                  \
        ((size_t)(align) & ((size_t)(align) - 1)) == 0,               \
        ((size_t)_bl_priv_align & ((size_t)_bl_priv_align - 1)) == 0, \
        "`align` must be a power of 2");                              \
    BL_PRIV_STMT_EXPR_RET_SUB                                         \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                       \
    register const ptrdiff_t _bl_priv_offs = (offs);                  \
    BL_PRIV_IASSERT((offs) >= 0, _bl_priv_offs >= 0,                  \
                    "`offs` must be non-negative");                   \
    BL_PRIV_IASSERT(                                                  \
        (size_t)(align) + (size_t)(offs) >= (size_t)(align),          \
        (size_t)_bl_priv_align + (size_t)_bl_priv_offs >= (size_t)_bl_priv_align, \
        "detected wrap-around; too large `align` and/or `offs`");     \
    BL_PRIV_STMT_EXPR_RET_SUB                                         \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                       \
    register const blsize _bl_priv_n = (n);                           \
    BL_PRIV_IASSERT((n) > 0 && (n) <= SIZE_MAX,                       \
                    _bl_priv_n > 0 && _bl_priv_n <= SIZE_MAX,         \
                    "`n` must be in (0, SIZE_MAX]");                  \
    BL_PRIV_STMT_EXPR_RET_SUB                                         \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                       \
    register const struct blayout *const _bl_priv_lays = (lays);      \
    BL_PRIV_IASSERT((lays) != NULL, _bl_priv_lays != NULL,            \
                    "`lays` must point to a non-zero-sized array");   \
    BL_PRIV_STMT_EXPR_RET_SUB                                         \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                       \
    register const blsize _bl_priv_prev_size = (prev_size);           \
    BL_PRIV_STMT_EXPR_RET bl_priv_calc(_bl_priv_align,                \
                                       _bl_priv_offs,                 \
                                       _bl_priv_n,                    \
                                       _bl_priv_lays,                 \
                                       _bl_priv_prev_size);           \
    BL_PRIV_STMT_EXPR_END_SUB                                         \
    BL_PRIV_STMT_EXPR_END_SUB                                         \
    BL_PRIV_STMT_EXPR_END_SUB                                         \
    BL_PRIV_STMT_EXPR_END_SUB                                         \
    BL_PRIV_STMT_EXPR_END

#if defined BL_CONST && BL_CONST >= 1
#define blnextc(ptr, curr_size, next_align)                                   \
    BL_PRIV_STMT_EXPR_BEGIN                                                   \
    register const void *const _bl_priv_ptr = (ptr);                          \
    BL_PRIV_IASSERT((ptr) != NULL, _bl_priv_ptr != NULL,                      \
                    "`ptr` cannot be NULL");                                  \
    BL_PRIV_STMT_EXPR_RET_SUB                                                 \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                               \
    register const blsize _bl_priv_curr_size = (curr_size);                   \
    register const blsize _bl_priv_next_align = (next_align);                 \
    BL_PRIV_IASSERT((next_align) > 0, _bl_priv_next_align > 0,                \
                    "`next_align` must be a power of 2");                     \
    BL_PRIV_IASSERT(                                                          \
        ((size_t)(next_align) & ((size_t)(next_align) - 1)) == 0,             \
        ((size_t)_bl_priv_next_align & ((size_t)_bl_priv_next_align - 1)) == 0, \
        "`next_align` must be a power of 2");                                 \
    BL_PRIV_STMT_EXPR_RET                                                     \
        bl_priv_nextc(_bl_priv_ptr, _bl_priv_curr_size, _bl_priv_next_align); \
    BL_PRIV_STMT_EXPR_END_SUB                                                 \
    BL_PRIV_STMT_EXPR_END
#define blprevc(ptr, prev_size, prev_align)                                   \
    BL_PRIV_STMT_EXPR_BEGIN                                                   \
    register const void *const _bl_priv_ptr = (ptr);                          \
    BL_PRIV_IASSERT((ptr) != NULL, _bl_priv_ptr != NULL,                      \
                    "`ptr` cannot be NULL");                                  \
    BL_PRIV_STMT_EXPR_RET_SUB                                                 \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                               \
    register const blsize _bl_priv_prev_size = (prev_size);                   \
    BL_PRIV_IASSERT((prev_size) > 0, _bl_priv_prev_size > 0,                  \
                    "`prev_size` must be in (0, SIZE_MAX]");                  \
    BL_PRIV_IASSERT((prev_size) <= SIZE_MAX, _bl_priv_prev_size <= SIZE_MAX,  \
                    "`prev_size` must be in (0, SIZE_MAX]");                  \
    BL_PRIV_STMT_EXPR_RET_SUB                                                 \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                               \
    register const blsize _bl_priv_prev_align = (prev_align);                 \
    BL_PRIV_IASSERT((prev_align) > 0, _bl_priv_prev_align > 0,                \
                    "`prev_align` must be a power of 2");                     \
    BL_PRIV_IASSERT(                                                          \
        ((size_t)(prev_align) & ((size_t)(prev_align) - 1)) == 0,             \
        ((size_t)_bl_priv_prev_align & ((size_t)_bl_priv_prev_align - 1)) == 0, \
        "`prev_align` must be a power of 2");                                 \
    BL_PRIV_STMT_EXPR_RET                                                     \
        bl_priv_prevc(_bl_priv_ptr, _bl_priv_prev_size, _bl_priv_prev_align); \
    BL_PRIV_STMT_EXPR_END_SUB                                                 \
    BL_PRIV_STMT_EXPR_END_SUB                                                 \
    BL_PRIV_STMT_EXPR_END
#define blmembc(obj, size, idx)                          \
    BL_PRIV_STMT_EXPR_BEGIN                              \
    register const void *const _bl_priv_obj = (obj);     \
    BL_PRIV_IASSERT((obj) != NULL, _bl_priv_obj != NULL, \
                    "`obj` can't be NULL");              \
    BL_PRIV_STMT_EXPR_RET_SUB                            \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                          \
    register const blsize _bl_priv_size = (size);        \
    BL_PRIV_IASSERT((size) > 0, _bl_priv_size > 0,       \
                    "`size` must be in (0, SIZE_MAX]");  \
    BL_PRIV_STMT_EXPR_RET_SUB                            \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                          \
    register const ptrdiff_t _bl_priv_idx = (idx);       \
    BL_PRIV_STMT_EXPR_RET bl_priv_membc(_bl_priv_obj,    \
                                        _bl_priv_size,   \
                                        _bl_priv_idx);   \
    BL_PRIV_STMT_EXPR_END_SUB                            \
    BL_PRIV_STMT_EXPR_END_SUB                            \
    BL_PRIV_STMT_EXPR_END
#endif

#if !defined BL_CONST || BL_CONST <= 1
#define blnext(ptr, curr_size, next_align)                                   \
    BL_PRIV_STMT_EXPR_BEGIN                                                  \
    register void *const _bl_priv_ptr = (ptr);                               \
    BL_PRIV_IASSERT((ptr) != NULL, _bl_priv_ptr != NULL,                     \
                    "`ptr` cannot be NULL");                                 \
    BL_PRIV_STMT_EXPR_RET_SUB                                                \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                              \
    register const blsize _bl_priv_curr_size = (curr_size);                  \
    register const blsize _bl_priv_next_align = (next_align);                \
    BL_PRIV_IASSERT((next_align) > 0, _bl_priv_next_align > 0,               \
                    "`next_align` must be a power of 2");                    \
    BL_PRIV_IASSERT(                                                         \
        ((size_t)(next_align) & ((size_t)(next_align) - 1)) == 0,            \
        ((size_t)_bl_priv_next_align & ((size_t)_bl_priv_next_align - 1)) == 0, \
        "`next_align` must be a power of 2");                                \
    BL_PRIV_STMT_EXPR_RET                                                    \
        bl_priv_next(_bl_priv_ptr, _bl_priv_curr_size, _bl_priv_next_align); \
    BL_PRIV_STMT_EXPR_END_SUB                                                \
    BL_PRIV_STMT_EXPR_END
#define blprev(ptr, prev_size, prev_align)                                   \
    BL_PRIV_STMT_EXPR_BEGIN                                                  \
    register void *const _bl_priv_ptr = (ptr);                               \
    BL_PRIV_IASSERT((ptr) != NULL, _bl_priv_ptr != NULL,                     \
                    "`ptr` cannot be NULL");                                 \
    BL_PRIV_STMT_EXPR_RET_SUB                                                \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                              \
    register const blsize _bl_priv_prev_size = (prev_size);                  \
    BL_PRIV_IASSERT((prev_size) > 0, _bl_priv_prev_size > 0,                 \
                    "`prev_size` must be in (0, SIZE_MAX]");                 \
    BL_PRIV_IASSERT((prev_size) <= SIZE_MAX, _bl_priv_prev_size <= SIZE_MAX, \
                    "`prev_size` must be in (0, SIZE_MAX]");                 \
    BL_PRIV_STMT_EXPR_RET_SUB                                                \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                              \
    register const blsize _bl_priv_prev_align = (prev_align);                \
    BL_PRIV_IASSERT((prev_align) > 0, _bl_priv_prev_align > 0,               \
                    "`prev_align` must be a power of 2");                    \
    BL_PRIV_IASSERT(                                                         \
        ((size_t)(prev_align) & ((size_t)(prev_align) - 1)) == 0,            \
        ((size_t)_bl_priv_prev_align & ((size_t)_bl_priv_prev_align - 1)) == 0, \
        "`prev_align` must be a power of 2");                                \
    BL_PRIV_STMT_EXPR_RET                                                    \
        bl_priv_prev(_bl_priv_ptr, _bl_priv_prev_size, _bl_priv_prev_align); \
    BL_PRIV_STMT_EXPR_END_SUB                                                \
    BL_PRIV_STMT_EXPR_END_SUB                                                \
    BL_PRIV_STMT_EXPR_END
#define blmemb(obj, size, idx)                           \
    BL_PRIV_STMT_EXPR_BEGIN                              \
    register void *const _bl_priv_obj = (obj);           \
    BL_PRIV_IASSERT((obj) != NULL, _bl_priv_obj != NULL, \
                    "`obj` can't be NULL");              \
    BL_PRIV_STMT_EXPR_RET_SUB                            \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                          \
    register const blsize _bl_priv_size = (size);        \
    BL_PRIV_IASSERT((size) > 0, _bl_priv_size > 0,       \
                    "`size` must be in (0, SIZE_MAX]");  \
    BL_PRIV_STMT_EXPR_RET_SUB                            \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                          \
    register const ptrdiff_t _bl_priv_idx = (idx);       \
    BL_PRIV_STMT_EXPR_RET bl_priv_memb(_bl_priv_obj,     \
                                       _bl_priv_size,    \
                                       _bl_priv_idx);    \
    BL_PRIV_STMT_EXPR_END_SUB                            \
    BL_PRIV_STMT_EXPR_END_SUB                            \
    BL_PRIV_STMT_EXPR_END
#elif defined BL_CONST && BL_CONST >= 2
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 201112L  /* C11 */
#define blnext(ptr, curr_size, next_align)                        \
    BL_PRIV_STMT_EXPR_BEGIN                                       \
    register void *const _bl_priv_ptr = BL_PRIV_UNCONST(ptr);     \
    BL_PRIV_IASSERT((ptr) != NULL, _bl_priv_ptr != NULL,          \
                    "`ptr` cannot be NULL");                      \
    BL_PRIV_STMT_EXPR_RET_SUB                                     \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                   \
    register const blsize _bl_priv_curr_size = (curr_size);       \
    register const blsize _bl_priv_next_align = (next_align);     \
    BL_PRIV_IASSERT((next_align) > 0, _bl_priv_next_align > 0,    \
                    "`next_align` must be a power of 2");         \
    BL_PRIV_IASSERT(                                              \
        ((size_t)(next_align) & ((size_t)(next_align) - 1)) == 0, \
        ((size_t)_bl_priv_next_align & ((size_t)_bl_priv_next_align - 1)) == 0, \
        "`next_align` must be a power of 2");                     \
    BL_PRIV_STMT_EXPR_RET                                         \
        _Generic(0 ? (ptr) : _bl_priv_ptr,                        \
                 void *:       bl_priv_next,                      \
                 const void *: bl_priv_nextc)                     \
        (_bl_priv_ptr, _bl_priv_curr_size, _bl_priv_next_align);  \
    BL_PRIV_STMT_EXPR_END_SUB                                     \
    BL_PRIV_STMT_EXPR_END
#define blprev(ptr, prev_size, prev_align)                                   \
    BL_PRIV_STMT_EXPR_BEGIN                                                  \
    register void *const _bl_priv_ptr = BL_PRIV_UNCONST(ptr);                \
    BL_PRIV_IASSERT((ptr) != NULL, _bl_priv_ptr != NULL,                     \
                    "`ptr` cannot be NULL");                                 \
    BL_PRIV_STMT_EXPR_RET_SUB                                                \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                              \
    register const blsize _bl_priv_prev_size = (prev_size);                  \
    BL_PRIV_IASSERT((prev_size) > 0, _bl_priv_prev_size > 0,                 \
                    "`prev_size` must be in (0, SIZE_MAX]");                 \
    BL_PRIV_IASSERT((prev_size) <= SIZE_MAX, _bl_priv_prev_size <= SIZE_MAX, \
                    "`prev_size` must be in (0, SIZE_MAX]");                 \
    BL_PRIV_STMT_EXPR_RET_SUB                                                \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                              \
    register const blsize _bl_priv_prev_align = (prev_align);                \
    BL_PRIV_IASSERT((prev_align) > 0, _bl_priv_prev_align > 0,               \
                    "`prev_align` must be a power of 2");                    \
    BL_PRIV_IASSERT(                                                         \
        ((size_t)(prev_align) & ((size_t)(prev_align) - 1)) == 0,            \
        ((size_t)_bl_priv_prev_align & ((size_t)_bl_priv_prev_align - 1)) == 0, \
        "`prev_align` must be a power of 2");                                \
    BL_PRIV_STMT_EXPR_RET                                                    \
        _Generic(0 ? (ptr) : _bl_priv_ptr,                                   \
                 void *:       bl_priv_prev,                                 \
                 const void *: bl_priv_prevc)                                \
        (_bl_priv_ptr, _bl_priv_prev_size, _bl_priv_prev_align);             \
    BL_PRIV_STMT_EXPR_END_SUB                                                \
    BL_PRIV_STMT_EXPR_END_SUB                                                \
    BL_PRIV_STMT_EXPR_END
#define blmemb(obj, size, idx)                           \
    BL_PRIV_STMT_EXPR_BEGIN                              \
    register void *const _bl_priv_obj = (obj);           \
    BL_PRIV_IASSERT((obj) != NULL, _bl_priv_obj != NULL, \
                    "`obj` can't be NULL");              \
    BL_PRIV_STMT_EXPR_RET_SUB                            \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                          \
    register const blsize _bl_priv_size = (size);        \
    BL_PRIV_IASSERT((size) > 0, _bl_priv_size > 0,       \
                    "`size` must be in (0, SIZE_MAX]");  \
    BL_PRIV_STMT_EXPR_RET_SUB                            \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                          \
    register const ptrdiff_t _bl_priv_idx = (idx);       \
    BL_PRIV_STMT_EXPR_RET                                \
        _Generic(0 ? (obj) : _bl_priv_obj,               \
                 void *:       bl_priv_memb,             \
                 const void *: bl_priv_membc)            \
        (_bl_priv_obj, _bl_priv_size, _bl_priv_idx);     \
    BL_PRIV_STMT_EXPR_END_SUB                            \
    BL_PRIV_STMT_EXPR_END_SUB                            \
    BL_PRIV_STMT_EXPR_END
#else
#define blnext(ptr, curr_size, next_align)                        \
    BL_PRIV_STMT_EXPR_BEGIN                                       \
    register void *const _bl_priv_ptr = BL_PRIV_UNCONST(ptr);     \
    BL_PRIV_IASSERT((ptr) != NULL, _bl_priv_ptr != NULL,          \
                    "`ptr` cannot be NULL");                      \
    BL_PRIV_STMT_EXPR_RET_SUB                                     \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                   \
    register const blsize _bl_priv_curr_size = (curr_size);       \
    register const blsize _bl_priv_next_align = (next_align);     \
    BL_PRIV_IASSERT((next_align) > 0, _bl_priv_next_align > 0,    \
                    "`next_align` must be a power of 2");         \
    BL_PRIV_IASSERT(                                              \
        ((size_t)(next_align) & ((size_t)(next_align) - 1)) == 0, \
        ((size_t)_bl_priv_next_align & ((size_t)_bl_priv_next_align - 1)) == 0, \
        "`next_align` must be a power of 2");                     \
    BL_PRIV_STMT_EXPR_RET (1 ? bl_priv_next(_bl_priv_ptr,         \
                                            _bl_priv_curr_size,   \
                                            _bl_priv_next_align)  \
                             : (ptr));                            \
    BL_PRIV_STMT_EXPR_END_SUB                                     \
    BL_PRIV_STMT_EXPR_END
#define blprev(ptr, prev_size, prev_align)                                   \
    BL_PRIV_STMT_EXPR_BEGIN                                                  \
    register void *const _bl_priv_ptr = BL_PRIV_UNCONST(ptr);                \
    BL_PRIV_IASSERT((ptr) != NULL, _bl_priv_ptr != NULL,                     \
                    "`ptr` cannot be NULL");                                 \
    BL_PRIV_STMT_EXPR_RET_SUB                                                \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                              \
    register const blsize _bl_priv_prev_size = (prev_size);                  \
    BL_PRIV_IASSERT((prev_size) > 0, _bl_priv_prev_size > 0,                 \
                    "`prev_size` must be in (0, SIZE_MAX]");                 \
    BL_PRIV_IASSERT((prev_size) <= SIZE_MAX, _bl_priv_prev_size <= SIZE_MAX, \
                    "`prev_size` must be in (0, SIZE_MAX]");                 \
    BL_PRIV_STMT_EXPR_RET_SUB                                                \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                              \
    register const blsize _bl_priv_prev_align = (prev_align);                \
    BL_PRIV_IASSERT((prev_align) > 0, _bl_priv_prev_align > 0,               \
                    "`prev_align` must be a power of 2");                    \
    BL_PRIV_IASSERT(                                                         \
        ((size_t)(prev_align) & ((size_t)(prev_align) - 1)) == 0,            \
        ((size_t)_bl_priv_prev_align & ((size_t)_bl_priv_prev_align - 1)) == 0, \
        "`prev_align` must be a power of 2");                                \
    BL_PRIV_STMT_EXPR_RET (1 ? bl_priv_prev(_bl_priv_ptr,                    \
                                            _bl_priv_prev_size,              \
                                            _bl_priv_prev_align)             \
                             : (ptr));                                       \
    BL_PRIV_STMT_EXPR_END_SUB                                                \
    BL_PRIV_STMT_EXPR_END_SUB                                                \
    BL_PRIV_STMT_EXPR_END
#define blmemb(obj, size, idx)                             \
    BL_PRIV_STMT_EXPR_BEGIN                                \
    register void *const _bl_priv_obj = (obj);             \
    BL_PRIV_IASSERT((obj) != NULL, _bl_priv_obj != NULL,   \
                    "`obj` can't be NULL");                \
    BL_PRIV_STMT_EXPR_RET_SUB                              \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                            \
    register const blsize _bl_priv_size = (size);          \
    BL_PRIV_IASSERT((size) > 0, _bl_priv_size > 0,         \
                    "`size` must be in (0, SIZE_MAX]");    \
    BL_PRIV_STMT_EXPR_RET_SUB                              \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                            \
    register const ptrdiff_t _bl_priv_idx = (idx);         \
    BL_PRIV_STMT_EXPR_RET (1 ? bl_priv_memb(_bl_priv_obj,  \
                                            _bl_priv_size, \
                                            _bl_priv_idx)  \
                             : (obj));                     \
    BL_PRIV_STMT_EXPR_END_SUB                              \
    BL_PRIV_STMT_EXPR_END_SUB                              \
    BL_PRIV_STMT_EXPR_END
#endif
#endif

#define blsizeof(l)                                                          \
    BL_PRIV_STMT_EXPR_BEGIN                                                  \
    register const struct blayout *const _bl_priv_l = (l);                   \
    BL_PRIV_IASSERT((l) != NULL, _bl_priv_l != NULL, "`l` cannot be NULL");  \
    BL_PRIV_STMT_EXPR_RET_SUB                                                \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                              \
    register const blsize _bl_priv_nmemb = _bl_priv_l->nmemb;                \
    BL_ASSERT(_bl_priv_nmemb > 0);                                           \
    BL_PRIV_STMT_EXPR_RET_SUB                                                \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                              \
    register const blsize _bl_priv_size = _bl_priv_l->size;                  \
    BL_ASSERT(_bl_priv_size > 0);                                            \
    BL_ASSERT(!(_bl_priv_nmemb > BL_SIZEMAX / _bl_priv_size));               \
    BL_ASSERT(_bl_priv_l->align > 0);                                        \
    BL_ASSERT(                                                               \
        ((size_t)_bl_priv_l->align & ((size_t)_bl_priv_l->align - 1)) == 0); \
    BL_PRIV_STMT_EXPR_RET _bl_priv_nmemb * _bl_priv_size;                    \
    BL_PRIV_STMT_EXPR_END_SUB                                                \
    BL_PRIV_STMT_EXPR_END_SUB                                                \
    BL_PRIV_STMT_EXPR_END

#define blaligned(size, align)                                             \
    BL_PRIV_STMT_EXPR_BEGIN                                                \
    register const blsize _bl_priv_size = (size);                          \
    BL_PRIV_IASSERT((size) > 0, _bl_priv_size > 0,                         \
                    "`size` must be in (0, SIZE_MAX]");                    \
    BL_PRIV_IASSERT((size) <= SIZE_MAX, _bl_priv_size <= SIZE_MAX,         \
                    "`size` must be in (0, SIZE_MAX]");                    \
    BL_PRIV_STMT_EXPR_RET_SUB                                              \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                            \
    register const blsize _bl_priv_align = (align);                        \
    BL_PRIV_IASSERT((align) > 0, _bl_priv_align > 0,                       \
                    "`align` must be a power of 2");                       \
    BL_PRIV_IASSERT(                                                       \
        ((size_t)(align) & ((size_t)(align) - 1)) == 0,                    \
        ((size_t)_bl_priv_align & ((size_t)_bl_priv_align - 1)) == 0,      \
        "`align` must be a power of 2");                                   \
    BL_PRIV_STMT_EXPR_RET_SUB                                              \
    BL_PRIV_STMT_EXPR_BEGIN_SUB                                            \
    register const size_t _bl_priv_r =                                     \
        (size_t) BLALIGNED((size_t)_bl_priv_size, (size_t)_bl_priv_align); \
    BL_PRIV_IASSERT((size_t) BLALIGNED((size_t)(size), (size_t)(align)) >= (size_t)(size), \
                    _bl_priv_r >= (size_t)_bl_priv_size,                   \
                    "detected wrap-around in `blaligned()`");              \
    BL_PRIV_IASSERT((size_t) BLALIGNED((size_t)(size), (size_t)(align)) <= (size_t)BL_SIZEMAX, \
                    _bl_priv_r <= (size_t)BL_SIZEMAX,                      \
                    "`blaligned()` result can't fit in `blsize`");         \
    BL_PRIV_STMT_EXPR_RET (blsize)_bl_priv_r;                              \
    BL_PRIV_STMT_EXPR_END_SUB                                              \
    BL_PRIV_STMT_EXPR_END_SUB                                              \
    BL_PRIV_STMT_EXPR_END

#endif

#undef BL_PRIV_INLINE_ALWAYS
#if !BL_PRIV_INLINE_USER
#undef BL_INLINE
#endif
#undef BL_PRIV_INLINE_USER
#undef BL_PRIV_ATTR

#endif  /* BLAYOUT_H */
