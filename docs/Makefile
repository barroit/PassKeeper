SOURCE = $(wildcard *.adoc)
HTML = $(SOURCE:.adoc=.html)
MANPAGE = $(SOURCE:.adoc=.1)

ASCIIDOC = asciidoctor
ASCIIDOC_MAN = $(ASCIIDOC) -amanmanual='PassKeeper Manual' \
		-amansource='PassKeeper ${PK_VERSION}' \
		-aauthor='${PK_AUTHOR}' -aemail='${PK_AUTHOR_CONTACT}' \
		-adoctype=manpage -aman-linkstyle='pass:[blue R < >]'

all: html manpage


html: $(HTML)

%.html: %.adoc
	$(ASCIIDOC) $<


manpage: $(MANPAGE)

%.1: %.adoc
	$(ASCIIDOC_MAN) -b manpage $<


clean:
	rm $(HTML) $(MANPAGE)

.PHONY: all manpage clean