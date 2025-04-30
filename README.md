<!-- Copyright 2025, pan (pan_@disroot.org) -->
<!-- SPDX-License-Identifier: MIT-0 -->
# BLayout

A small C header to help with laying out and retrieving heterogeneous objects in contiguous memory.

# Example
```c
const struct blayout lays[] = {{1, sizeof(double), alignof(double)},
                               {1, sizeof(int),    alignof(int)   }};
size_t size = blcalc(BL_ALIGNMENT, 0, 2, lays, 0);
assert(size != 0 && "`blcalc()` error");

void *block = malloc(size);
assert(block != NULL && "`malloc()` error");

double *d = blnext(block,                  0, lays[0].align);
int    *i = blnext(    d, blsizeof(&lays[0]), lays[1].align);

*d = 3.14;
*i = 42;
printf("%f %d\n", *d, *i);

/*
 * `BL_ALIGNMENT` is equal to `alignof(max_align_t)` but the header provides
 * a portable alternative, because MSVC, while it claims C11 conformance,
 * doesn't have `max_align_t`, even though the standard mandates it. See
 * comment in the header for more information.
 */
```

# Dependencies
Only freestanding headers are required:

* `<assert.h>` _(optional)_
* `<stddef.h>`
  - `max_align_t` _(optional)_
  - `ptrdiff_t`
  - `size_t` _(overridable)_
* `<stdint.h>`
  - `SIZE_MAX` _(overridable)_
  - `uintptr_t`

Currently, the header targets C99 and later standards, due to the hard dependency on the existence of `uintptr_t`. Other than that, you should be able to compile the code with any C89/C90/ANSI compiler if you provide a suitable substitute for your platform.

The header may or may not compile and work under a C++ compiler.

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
