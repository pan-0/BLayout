`DOCS.md` is checked in the repository for convenience. If you want to generate
it yourself, you'll need [`m4`](https://en.wikipedia.org/wiki/M4_(computer_language)) and [`make`](https://en.wikipedia.org/wiki/Make_(software)). Only the GNU implementations of the above programs have been tested.

For `DOCS.md`:
```console
$ make
```

For generating a PDF file (`DOCS.pdf`), you'll also need [`pandoc`](https://pandoc.org/).
```console
$ make pdf
```