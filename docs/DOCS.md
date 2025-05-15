---
title: BLayout Documentation
date: May 1, 2025
author: pan <pan_@disroot.org>
---
<!-- Copyright 2025, pan (pan_@disroot.org) -->
<!-- SPDX-License-Identifier: MIT-0 -->

# BLayout Documentation
1. [API](#api)
   1. [Types](#types)
   2. [Constants](#constants)
   3. [Macros](#macros)
   4. [Functions](#functions)
2. [Terminology](#terminology)
3. [Usage](#usage)
   1. [`blcalc()` with `blnext()`](#blcalc-with-blnext)
   2. [`blprev()`](#blprev)
   3. [`blnext()` vs. `blprev()`](#blnext-vs-blprev)
   4. [`blcalc()` chaining](#blcalc-chaining)
3. [LICENSE](#license)

# API
1. [Types](#types)
2. [Constants](#constants)
3. [Macros](#macros)
4. [Functions](#functions)

## Types
```c
typedef uintptr_t bluptr;
typedef size_t    blsize;

struct blayout {
	blsize nmemb;
	blsize size;
	blsize align;
};
```
* `bluptr` is used internally to cast `void *` pointers to an integer type, where arithmetic may be performed. This is required for returning properly aligned pointers and such. Since the default, `uintptr_t`, is only available from C99 onwards, this `typedef` is provided to ease porting when using an earlier C standard and/or implementations where such a type is not offered. The header assumes that casting a `void *` pointer to `uintptr_t` leaves the bits unchanged or zero-extends, in case the latter is wider. A round-trip conversion, using the types above, is guaranteed by the C standard to result to a pointer referencing the same object as the original pointer. These semantics match the implementations offered by [GCC](https://gcc.gnu.org/onlinedocs/gcc/Arrays-and-pointers-implementation.html) and Clang.
* `blsize` is the API's size type. It's `size_t` by default. You may change this type by modifying BLayout's header. A `signed` type is also valid. You'd have to change `BL_SIZEMAX` accordingly (see [below](#constants)).
* `blayout` describes a layout for a single object, where:
  - `nmemb` is the number of elements this object will hold (like `calloc()`'s first argument),
  - `size` is the size (in bytes) of each element/type (like `calloc()`'s second argument),
  - `align` is the alignment[^1] of the object's type

## Constants
```c
#define BL_SIZEMAX   SIZE_MAX
#define BL_ALIGNMENT alignof(max_align_t)
```
* `BL_SIZEMAX` is the equivalent to `size_t`'s `SIZE_MAX` for `blsize` and is equal to that by default. You may override this, but the header assumes that it's **greater** than $0$.
* `BL_ALIGNMENT` is never used internally. It's equal to the maximum alignment[^1] among C's scalar types. It's provided as a convenience when calling `blcalc()` (see [below](#functions)). This, too, can be overridden.

_Note: To override these, either modify BLayout's header or `#define` them **before** including `blayout.h`._

## Macros
```c
#define BL_API    static
#define BL_ASSERT assert
#define BL_INLINE inline
#define BL_DEBUG  0
#define BL_CONST  0
```
* `BL_API` is currently only used as a visual aid, do **not** try to change it.
* BLayout can use assertions through the `BL_ASSERT` macro to enforce API contracts and prevent footguns. You can override this macro if you use a custom `assert()` function. See `BL_DEBUG` below if you want to disable assertions.
* Every function is `inline` (C99 [semantics](https://lists.llvm.org/pipermail/llvm-dev/2021-August/152031.html)) through the `BL_INLINE` macro. This is so that you can workaround C's deficiencies, if you so wish.
* `BL_DEBUG` can be defined to four possible values:
  - $0$, where BLayout will use **no** assertions (see above) and, in addition, will try to use compiler-specific annotations (e.g. `attribute(nonnull(...))`) in a portable and non-intrusive manner. This is the default.
  - $1$, where BLayout will use **some** assertions and annotations.
  - $2$, where BLayout will use **all** assertions **and no** annotations.
  - $3$, where the behavior is identical to $2$, but BLayout will try to use inline assertions instead, which would potentially - depending on the implementation - show better error messages if triggered. Additionally, BLayout will try to statically determine if function input arguments are valid at compile time. This option is meant to be used only during development and is only supported under GCC and Clang compilers.
* `BL_CONST` can be used to include `const`-aware functions (see [below](#functions)). It's not defined by default, but can be to four possible values:
  - $0$, where no `const`-aware functions will be included. The behavior is the same as if `BL_CONST` wasn't defined. This is the default.
  - $1$, where BLayout will include `const`-aware functions (`blnextc()`, `blprevc()`; see [below](#functions)).
  - $2$, where BLayout will change `blnext()` and `blprev()` to automatically and correctly handle the `const`-qualified case of input pointers, as well as the non-qualified case.
  - $3$, where the behavior is identical to $2$, but also compatible with the `-Wcast-qual` warning offered by [GCC](https://gcc.gnu.org/onlinedocs/gcc-15.1.0/gcc/Warning-Options.html#index-Wcast-qual) and [Clang](https://clang.llvm.org/docs/DiagnosticsReference.html#wcast-qual).

## Functions
_Note: Reading the [terminology](#terminology) section first might clear up some terms that are used in the descriptions below._
```c
BL_API blsize blcalc(blsize align,
                     ptrdiff_t offs,
                     blsize n,
                     const struct blayout *lays,
                     blsize prev_size);

BL_API void *blnext(void *ptr, blsize curr_size, blsize next_align);
BL_API void *blprev(void *ptr, blsize prev_size, blsize prev_align);

BL_API blsize blsizeof(const struct blayout *l);

#if BL_CONST >= 1
BL_API const void *blnextc(const void *ptr, blsize curr_size, blsize next_align);
BL_API const void *blprevc(const void *ptr, blsize prev_size, blsize prev_align);
#endif
```
* `blcalc()` returns the minimum size needed to contiguously lay out multiple objects. The function assumes that all arguments are valid and within bounds. If wrap-around is detected when computing the size, $0$ is returned instead.
  - `align` is the default alignment[^1] your allocator supports. In case you already have an allocated block, pass the block's alignment. `BL_ALIGNMENT` should be compatible with the default alignment of every memory block allocated by `malloc()` and every naturally-aligned[^2] type.
  - `offs` is used in case you want to lay out starting from a specific offset into a block. Pass $0$ otherwise.
  - `n` is the number of layouts. Should be **greater** than $0$.
  - `lays` is an array of length `n` containing layouts.
  - `prev_size` is used to chain multiple `blcalc()` calls. When first invoking, $0$ must be passed, otherwise the result of the previous `blcalc()` call must be passed, assuming the call succeeded and a **non**-$0$ value was returned. `align` and `offs` must not change across any chained calls.
* `blnext()` lays out the next object in a **left-to-right** manner, where:
  - `ptr` is a pointer to the most recent laid-out object. When first invoking, pass a pointer to your block (or `block + offs` if `blcalc()` was passed a non-zero `offs`). It's assumed to **not** be `NULL` and thus the function doesn't check for this.
  - `curr_size` is the size of the most recent object (see `blsizeof()`) and is assumed to be valid. When first invoking, pass $0$.
  - `next_align` is the alignment[^1] of the next object's type and is assumed to be valid. When first invoking, pass the alignment of the **first** object's type.
* `blprev()` is like `blnext()`, but lays out and returns the previous object, in a **right-to-left** manner. That means you should lay out in **reverse** order, starting with **last** object.
  - `ptr` is a pointer to the most recent laid-out object. When first invoking, pass a pointer to the **end** of your block. It's assumed to **not** be `NULL` and thus the function doesn't check for this.
  - `prev_size` is the size of the previous object (see `blsizeof()`) and is assumed to be valid. When first invoking, pass the size of the **last** object.
  - `prev_align` is the alignment[^1] of the previous object's type and is assumed to be valid. When first invoking pass the alignment of the **last** object's type.
* `blsizeof()` returns the total size (in bytes) of an object described by its layout. Effectively, it multiplies `blayout.nmemb` with `blayout.size`. It's provided as a convenience.
  - `l` is the pointer to the aforementioned layout.
  1. _Caveat: Padding due to alignment is **not** taken into account._
  2. _Caveat: Potential integer overflow is **not** checked. The layout is assumed to be correct. `blcalc()` already checks for this._
* `blnextc()` and `blprevc()` have identical behavior to `blnext()` and `blprev()` respectively. They are _not_ included if `BL_CONST` is undefined or has a value of $0$. They return and take a `const`-qualified pointer. Remember also that `blnext()` and `blprev()` can automatically preserve `const`-correctness if `BL_CONST` is defined to a value of $2$ or $3$.

Keep in mind that the signatures above are for reference. The actual implementation may significantly differ. For example, some functions may be implemented as a macro, meaning that you can't take their address. However, it's guaranteed that all arguments will be evaluated, and each will be evaluated once. Further, you can be assured that your lexical scope won't be polluted.

[^1]: [**alignment**](https://en.wikipedia.org/wiki/Data_structure_alignment) is _always assumed to be valid_: (1) it denotes _byte_ boundaries and (2) is a power of $2$.
[^2]: Meaning, every type that is not _over-aligned_: that does **not** have [extended alignment](https://port70.net/~nsz/c/c11/n1570.html#6.2.8p3).

# Terminology
Note that the terms **first**, **last**, **next** and **previous** object, apply in the context of its layout's position in the `lays` array. As an example, given this layouts array:
```c
const struct blayout x_layout = {4, sizeof(int),   alignof(int)  };
const struct blayout y_layout = {1, sizeof(char),  alignof(char) };
const struct blayout z_layout = {3, sizeof(float), alignof(float)};

#define N 3
const struct blayout lays[N] = {x_layout, y_layout, z_layout};
```

Meaning:

1. Four ($4$) integers (`int`).
2. One ($1$) `char`.
3. Three ($3$) single-precision floating point numbers (`float`).

The layout of the **first** object is `lays[0]` or, equivalently, `x_layout`.

The layout of the **last** object is `lays[2]` or, equivalently, `z_layout`.

Given the layout `lays[0]`, the **next** layout is `lays[1]`. The **previous** layout is _undefined_, since `lays[0]` is also the first layout.

Given the layout `lays[1]`, the **next** layout is `lays[2]`. The **previous** layout is `lays[0]`.

Given the layout `lays[2]`, the **next** layout is _undefined_, since `lays[2]` is also the last layout. The **previous** layout is `lay[1]`.

In table format:

| $Layout$ | $Next$ $(Lays_{i+1})$ | $Previous$ $(Lays_{i-1})$ | $First$ $(Lays_0)$ | $Last$ $(Lays_{N-1})$ |
| -------- | --------------------- | ------------------------- | ------------------ | --------------------- |
| $Lays_0$ | $Lays_1$              | $Undefined$               | $Yes$              |                       |
| $Lays_1$ | $Lays_2$              | $Lays_0$                  |                    |                       |
| $Lays_2$ | $Undefined$           | $Lays_1$                  |                    | $Yes$                 |

Laying out in a **left-to-right** manner means, informally, starting with a layout, then taking its next, the next after that and so on. Potentially up until and including the last layout.

Similarly, laying out in a **right-to-left** manner or, equivalently, laying out **in reverse** (**order**), means starting with a layout, then taking its previous, the previous after that and so on. Potentially up until and including the first object.

# Usage
1. [`blcalc()` with `blnext()`](#blcalc-with-blnext)
2. [`blprev()`](#blprev)
3. [`blnext()` vs. `blprev()`](#blnext-vs-blprev)
4. [`blcalc()` chaining](#blcalc-chaining)

## `blcalc()` with `blnext()`
```c
/*
 * Request:
 *    I. One `int`, naturally aligned.
 *   II. Two `float`s, naturally aligned.
 *
 * The order of the `lays` array is important! `blcalc()` takes the order into
 * account when computing its result. And it does that for the simple reason
 * that the whole point of this header is to allow the programmer to lay out
 * their objects in memory exactly how they want. Hence, we preserve the order,
 * because it might be important, we wouldn't know. If the order is _not_
 * important to _you_, this detail doesn't impair you.
 *
 * This order also defines how `blnext()` (and `blprev()`; see below) should be
 * called. The functions don't check for this, the burden, unfortunately, falls
 * onto the programmer.
 */
const struct blayout lays[] = {{1, sizeof(int),   alignof(int)  },
                               {2, sizeof(float), alignof(float)}};

size_t size = blcalc(BL_ALIGNMENT,  /* Going to use the default alignment. */
                     0,             /* Laying out starting from the `0`th position. */
                     2,             /* The number of layouts. */
                     lays,          /* The array of layouts describing our objects. */
                     0);            /* We aren't chaining `blcalc()` calls. */
if (size == 0) {
	fprintf(stderr, "blcalc() error\n");
	return EXIT_FAILURE;
}

void *block = malloc(size);
if (block == NULL) {
	fprintf(stderr, "malloc() error\n");
	return EXIT_FAILURE;
}

/* The first object. */
int *i = blnext(block,           /* First invocation: pass the block. */
                0,               /* First invocation: pass `0`. */
                lays[0].align);  /* Pass the alignment of the first object's type (`int`). */
assert(i != NULL);  /* Guaranteed to _not_ be `NULL`. */

/* Continue with the next object... */
float *f = blnext(i,                   /* Pass the most recent laid-out object. */
                  blsizeof(&lays[0]),  /* Pass the size of the most recent laid-out object. */
                  lays[1].align);      /* Pass the alignment of the next object's type (`float`). */
assert(f != NULL);  /* Likewise. */

/* Use `i` and `f` normally. */
*i = 42;
f[0] = 2.71;
f[1] = 3.14;
printf("*i=%d f[0]=%f f[1]=%f\n", *i, f[0], f[1]);

/* Cleanup. */
free(block);  /* `i` and `f` are guaranteed to be cleaned up with a _single_ `free()`. */
return EXIT_SUCCESS;

/*
 * Alternatively, if the alignment of the _first_ object's type is
 * _less-or-equal_ to your block's alignment, then the pointer to the first
 * laid-out object is equivalent to a pointer to the block.
 *
 * In our case:
 *
 *  1. `malloc()` must, as mandated by the C standard, be able to return a
 *     suitably aligned pointer for _every_ naturally-aligned type. That
 *     encompasses ours: `int`.
 *  2. `BL_ALIGNMENT` is, as stated, compatible with the alignment of every
 *     naturally-aligned type, which includes the alignment of the first
 *     object's type: `alignof(int)`.
 *  2. `BL_ALIGNMENT` is also compatible with the alignment of every memory
 *     block allocated by `malloc()`.
 *  4. Thus, we can conclude that `alignof(int) <= BL_ALIGNMENT`.
 *
 * Note that any type can be over-aligned. This is fine, because an extended
 * alignment guarantees natural alignment. Or, in the words of the standard[1]:
 *
 *   In general, the concept "correctly aligned" is transitive: if a pointer
 *   to type A is correctly aligned for a pointer to type B, which in turn is
 *   correctly aligned for a pointer to type C, then a pointer to type A is
 *   correctly aligned for a pointer to type C.
 *
 * [1]: <https://port70.net/~nsz/c/c11/n1570.html#note68>
 */
static_assert(alignof(int) <= BL_ALIGNMENT, "incorrect alignment");  /* Just to be sure. */
free(i);  /* Same as `free(block);` _in our case_. See the above comment why and when this holds true. */
return EXIT_SUCCESS;

/*
 * The above observation also implies that we could skip the first call to
 * `blnext()` and lay out the first object (`i`) like so:
 */
int *i = block;  /* Okay, as long as `alignof(int) <= BL_ALIGNMENT`. */

/* With the rest of the code being identical... */
```

## `blprev()`
Usage of `blprev()` is similar to the usage of [`blnext()`](#blcalc-with-blnext) with these notable differences:

1. You must lay out in **reverse** order.
2. In order to cleanup safely, you **must** retain a pointer to your block.

Let's see with a similar example:
```c
/*
 * ... Same layouts and boilerplate as the `blnext()` example...
 *
 * Assume `block` of size `size` has already been allocated as above.
 */
size_t size;
void *block;

/*
 * Lay out in **reverse** order, starting with the **last** object. `blprev()`
 * works in a _right-to-left_ manner; we have to pass the _end_ of our block
 * in the first invocation.
 */
void *end = (char *)block + size;
float *f = blprev(end,                 /* First invocation: pass the end of the block. */
                  blsizeof(&lays[1]),  /* First invocation: pass the size of the last object. */
                  lays[1].align);      /* First invocation: pass the alignment of the last object's type. */
assert(f != NULL);  /* Always true, same as before. */

/* Continue with the previous object... */
int *i = blprev(f,                   /* Pass the most recent laid-out object. */
                blsizeof(&lays[0]),  /* Pass the size of the previous object. */
                lays[0].align);      /* Pass the alignment of the previous object's type. */
assert(i != NULL);  /* Likewise. */

*i = 1337;
f[0] = 1.41;
f[1] = 1.61;
printf("*i=%d f[0]=%f f[1]=%f\n", *i, f[0], f[1]);

/*
 * Cleanup!
 *
 * NOTE: You can _not_ pass `i` or `f` in the case of `blprev()`! You must pass
 *       the pointer returned to you by `malloc()`!
 */
free(block);
return EXIT_SUCCESS;

//free(i);  /* XXX: Don't do this _ever_! */
//free(f);  /* XXX: Or this either! */
```

## `blnext()` vs. `blprev()`
When should you use the one or the other? Given the limitations of `blprev()`, shouldn't you always use `blnext()`? It depends. Firstly, you indeed **can only use one**[^3] of the two to lay out from a single memory region. Given that, which to pick?

Let's assume your layouts array is `{{1, 1, 1}, {1, 2, 2}}`. Meaning:

1. One ($1$) type of size $1$ (bytes) at an $1$-byte alignment boundary, and
2. One ($1$) type of size $2$ (bytes) at an $2$-byte alignment boundary.

Assuming your block is $16$-byte aligned, `blcalc()` (correctly) returns $4$ for the above array.

If your block sits at address $16$ then, using `blnext()` and starting from the beginning of the block, the bytes would be laid out like this:

| $Address$ | $16$  | $17$  | $18$  | $19$  |
| --------- | ----- | ----- | ----- | ----- |
| $Byte$    | $X_0$ |       | $Y_0$ | $Y_1$ |

But, if you were to use `blprev()`, starting from the end of the block, they'd be laid out like this:

| $Address$ | $16$  | $17$  | $18$  | $19$  |
| --------- | ----- | ----- | ----- | ----- |
| $Byte$    |       | $X_0$ | $Y_0$ | $Y_1$ |

Where $X_i$ is the $i$ th byte of object $X$, respectively $Y$.

Thus, the two methods give different results. This example also makes clear why you need to retain a pointer to your block in the case of `blprev()`: $X_0$ doesn't sit at the (original) address $16$!

**Always use `blnext()`**, unless you desire the special properties of `blprev()` and the limitations don't affect you. Here's two scenarios where that could be true:

* You always carry around a pointer to your block, so using `blprev()` has no extra burden. _Note that `blprev()` is generally 1-2 machine instructions shorter than `blnext()`, potentially depending on the ABI, compiler and optimization options. ([Related](https://fitzgen.com/2019/11/01/always-bump-downwards.html))._
* You depend on the layout `blprev()` gives. This happens when giving a "header" to a "payload", just like `malloc()` does.
  ```c
  struct header {
      int id;
  };

  struct payload {
      size_t size;
      unsigned char data[];  /* Flexible array member: <https://gustedt.wordpress.com/2011/03/14/flexible-array-member/> */
  };

  struct payload *new_payload(my_ctx *ctx, size_t payload_size)
  {
      const struct blayout lays[] = {
          {1, sizeof(struct header), alignof(struct header)},
          {1, offsetof(struct payload, data) + payload_size, alignof(struct payload)}
      };
      size_t size = blcalc(/* ... */, 0, 2, lays, 0);
      if (size == 0)
          abort();

      void *block_end = /* ... */;
      struct payload *p = blprev(block_end, blsizeof(&lays[1]), lays[1].align);
      p->size = payload_size;
      memset(p->data, 0, payload_size);

      struct header *h = blprev(p, blsizeof(&lays[0]), lays[0].align);
      h->id = /* ... */;

      return p;
  }

  void recycle_payload(my_ctx *ctx, struct payload *p)
  {
      struct header *h = blprev(p, sizeof(*h), alignof(struct header));  /* Okay. */
      int id = h->id;
      /* Do something with `id`... */
  }
  ```
  The above example **cannot** work with `blnext()`, it **only works with `blprev()`**. Even if you only used `blnext()` to lay out the objects, once you got to retrieve the header in:

  ```c
  struct header *h = blprev(p, sizeof(*h), alignof(struct header));  /* I'm sure this is perfectly fine, what could possibly go wr */
  int id = h->id;  /* Kaboom! */
  ```
  your computer would explode. You can't do this! _Even if it works_, you can't depend on this if you lay out using `blnext()`. Use `blprev()` instead. Remember that the two functions _lay out objects differently_.

[^3]: Which also means you can't use one function to retrieve objects in reverse order, if you laid them out using the other.

## `blcalc()` chaining
```c
/* Assuming these layouts and computed size again: */
const struct blayout lays[] = {{1, sizeof(int),   alignof(int)  },
                               {2, sizeof(float), alignof(float)}};
size_t size = blcalc(BL_ALIGNMENT, 0, 2, lays, 0);
assert(size != 0 && "`blcalc()` error");

/* We can compute the same result by chaining two `blcalc()` calls: */
size_t tmp = blcalc(BL_ALIGNMENT, 0, 1, lays, 0);
assert(tmp != 0 && "`blcalc()` error");
size_t chained_size = blcalc(BL_ALIGNMENT, 0, 1, lays + 1, tmp);
assert(chained_size != 0 && "`blcalc()` error");

/* If no wrap-around happened, this assertion is guaranteed to be true: */
assert(size == chained_size);

/*
 * Chaining generalizes to any number of calls, where the next immediate call
 * gets passed the result of the most recent call.
 */
size_t tmp_0 = blcalc(alignment, offset, ...,     0); if (tmp_0 == 0) error();
size_t tmp_1 = blcalc(alignment, offset, ..., tmp_0); if (tmp_1 == 0) error();
size_t tmp_2 = blcalc(alignment, offset, ..., tmp_1); if (tmp_2 == 0) error();
...;
size_t tmp_n = blcalc(alignment, offset, ..., tmp_{n-1});
if (tmp_n == 0)
	error();

size_t size = tmp_n;
```

# LICENSE
```
MIT No Attribution

Copyright 2025 pan <pan_@disroot.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
