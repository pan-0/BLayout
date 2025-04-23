A ~200 line C header to help with laying out and retrieving heterogeneous objects in contiguous memory.

# Example
```c
#include "blayout.h"
#include <assert.h>    /* static_assert */
#include <stdalign.h>  /* alignof */
#include <stddef.h>    /* size_t, NULL */
#include <stdlib.h>    /* malloc(), free() */
#include <stdio.h>     /* printf() */

#define lenof(A) (sizeof (A) / sizeof (A)[0])

struct Base {
	int x;
};

struct Derived {
	struct Base b;
	int y;
};

struct Wrapper {
	int z;
	//struct Base b;
};

/* To be sure that `struct Wrapper` is allocated at the base. */
static_assert(alignof(struct Wrapper) <= BL_ALIGNMENT, "");

int main(void)
{
	const struct blayout l[] = {
		{1, sizeof(struct Wrapper), alignof(struct Wrapper)},
		{1, sizeof(struct Derived), alignof(struct Derived)}
	};
	size_t size = blcalc(BL_ALIGNMENT, 0, lenof(l), l, 0);
	struct Wrapper *w = size == 0 ? NULL : malloc(size);
	if (w == NULL)
		return 1;

	/* ok */
	w->z = 1;

	/* ok */
	struct Base *b = blnext(w, blsizeof(&l[0]), l[1].align);
	b->x = 2;

	/* ok! */
	struct Derived *d = (struct Derived *)b;
	d->y = 3;

	printf("w->z=%d b->x=%d d->y=%d\n", w->z, b->x, d->y);

	d->b.x = 4;
	printf("w->z=%d b->x=%d d->y=%d\n", w->z, b->x, d->y);

	free(w);
	return 0;
}
```
Staying faithful to the irony of a modern production compiler, [Clang](https://clang.llvm.org) fails to produce identical code with the [UB](https://en.wikipedia.org/wiki/Undefined_behavior#Examples_in_C_and_C++)-variant of the above example and requires the following patch:
```diff
@@ -42 +42,9 @@
-	struct Base *b = blnext(w, blsizeof(&l[0]), l[1].align);
+	struct Base *b;
+#	ifdef __clang__
+		/* Clang fails to optimize `blnext()`, so we have to do a little bit more work. */
+		static_assert(alignof(struct Wrapper) == alignof(struct Derived) &&
+		             (sizeof(struct Wrapper) & (alignof(struct Wrapper) - 1)) == 0, "");
+		b = (struct Base *)((char *)w + sizeof(*w));
+#	else
+		b = blnext(w, blsizeof(&l[0]), l[1].align);
+#	endif
```
See also [here](https://reddit.com/r/C_Programming/comments/1age6gt).

# Dependencies
* `<assert.h>` _(optional)_
* `<stddef.h>`
  - `size_t`
  - `ptrdiff_t`
* `<stdint.h>`
  - `uintptr_t`
  - `SIZE_MAX` _(overridable)_

Currently, the header targets C99 and later standards, due to the hard dependency on the existence of `uintptr_t`. Other than that, you should be able to compile the code with any C89/C90/ANSI compiler if you provide a suitable substitute for your platform.

The code is accepted from major C++ compilers, but no tests for functionality, edge cases and undefined behavior have been performed.

# Documentation
See [here](docs/DOCS.md).

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
