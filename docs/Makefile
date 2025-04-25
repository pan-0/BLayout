# Copyright 2025, pan (pan_@disroot.org)
# SPDX-License-Identifier: MIT-0

.POSIX:
.SUFFIXES:
PANDOC_FLAGS ::= \
	--listings \
	--pdf-engine=lualatex \
	-V geometry:margin=1cm \
	-V geometry:foot=0cm \
	-V mainfont:FreeSans \
	-V sansfont:FreeSans \
	-V monofont:FreeMono \
	-V colorlinks=true \
	-V linkcolor=blue \
	-V urlcolor=red \
	-V toccolor=gray \
	--from markdown

all: DOCS.md
.PHONY: all

pdf: DOCS.pdf
.PHONY: pdf

DOCS.md: DOCS.m4
	m4 $< > $@

DOCS.pdf: docs-pandoc.md listings-setup.tex
	pandoc $< -H listings-setup.tex $(PANDOC_FLAGS) -o $@

docs-pandoc.md: DOCS.m4
	m4 -Dpandoc $< > $@

clean:
	@rm -f docs-pandoc.md DOCS.pdf
.PHONY: clean

cleanall: clean
	@rm -f DOCS.md
.PHONY: cleanall
