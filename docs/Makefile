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

# I'm _not_ happy with this.
EXIFTOOL_FLAGS ::= \
	-XMP-dc:Rights="This work is licensed under the \"MIT No Attribution\" (MIT-0) License" \
	-XMP-cc:AttributionName="pan" \
	-XMP-cc:License="https://opensource.org/license/mit-0" \
	-XMP-cc:Permits+="Derivative Works" \
	-XMP-cc:Permits+="Distribution" \
	-XMP-cc:Permits+="Reproduction" \
	-XMP-cc:Permits+="Sharing" \
	-XMP-pdf:Copyright="Copyright 2025, pan" \
	-XMP-pur:Copyright="Copyright 2025, pan" \
	-XMP-prism:CopyrightYear="2025" \
	-overwrite_original

# Ain't that confusing?
all: DOCS.md
.PHONY: all

pdf: DOCS.pdf
.PHONY: pdf

# Yup.
both: DOCS.md DOCS.pdf
.PHONY: both

DOCS.md: DOCS.m4
	m4 $< > $@

DOCS.pdf: docs-pandoc.md listings-setup.tex copyright.txt
	$(info This may take awhile...)
	pandoc $< -H listings-setup.tex $(PANDOC_FLAGS) -o $@ && \
	exiftool -@ copyright.txt $(EXIFTOOL_FLAGS) $@

docs-pandoc.md: DOCS.m4
	m4 -Dpandoc $< > $@

clean:
	@rm -f docs-pandoc.md DOCS.pdf
.PHONY: clean

cleanall: clean
	@rm -f DOCS.md
.PHONY: cleanall
