<!-- Copyright 2025, pan (pan_@disroot.org) -->
<!-- SPDX-License-Identifier: MIT-0 -->

`DOCS.md` is checked in the repository for convenience. If you want to generate
it yourself, you'll need [`m4`](https://en.wikipedia.org/wiki/M4_(computer_language)) and [`make`](https://en.wikipedia.org/wiki/Make_(software)). Only the [GNU](https://www.gnu.org/software/software.html) implementations of the above programs have been tested (versions `1.4.18` and `4.3` respectively).

For `DOCS.md`:
```console
$ make
```

For generating a [PDF](https://en.wikipedia.org/wiki/PDF) file (`DOCS.pdf`), you'll also need [`pandoc`](https://pandoc.org/) (`2.9.2.1`+), [`exiftool`](https://exiftool.org/) (`12.40`+) and [GNU FreeFont](https://www.gnu.org/software/freefont/). Keep in mind that `pandoc` itself will make use of [LuaTeX](https://www.luatex.org/), so you must have that installed too.
```console
$ make pdf
```
