GIT_DESC := $(shell git describe --tags)

DOXYGEN_CONF := axiom.doxyfile
CLEAN_DIR := html latex
ARCHIVE_DIR := archive
PDF_OUTPUT := $(GIT_DESC)-doxygen.pdf
HTML_OUTPUT := $(GIT_DESC)-doxygen-html


.PHONY: all clean doxygen archive pdf



all: pdf

doxygen:
	doxygen $(DOXYGEN_CONF)

pdf: doxygen
	cd latex && make

archive: pdf
	mkdir -p $(ARCHIVE_DIR)
	cp latex/refman.pdf $(ARCHIVE_DIR)/$(PDF_OUTPUT)
	tar -zcvf $(HTML_OUTPUT).tgz html --transform s/html/$(HTML_OUTPUT)/
	mv $(HTML_OUTPUT).tgz $(ARCHIVE_DIR)/


clean:
	rm -rf $(CLEAN_DIR)
