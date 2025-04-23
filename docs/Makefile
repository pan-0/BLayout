# Copyright 2025, pan (pan_@disroot.org)
# SPDX-License-Identifier: MIT-0

.POSIX:
.SUFFIXES:
CC := pandoc
CFLAGS := \
	--listings \
	-H listings-setup.tex \
	--pdf-engine=lualatex \
	-V geometry:margin=1cm \
	-V geometry:foot=0cm \
	-V 'mainfont:FreeSans' \
	-V 'sansfont:FreeSans' \
	-V 'monofont:FreeMono' \
	-V colorlinks=true \
	-V linkcolor=blue \
	-V urlcolor=red \
	-V toccolor=gray \
	--from markdown

all: DOCS.pdf
.PHONY: all

DOCS.pdf: docs-pandoc.md
	$(CC) $< $(CFLAGS) -o $@

clean:
	@rm -f DOCS.pdf
