FILES = axiom_nic_datasheet.pdf

all: $(FILES)

%.pdf: %.tex
	pdflatex $(*F) && pdflatex $(*F)

clean:
	rm -f $(FILES)
	rm -f *.log *.dvi *.aux *.nav *.out *.ps *.snm *.toc *.vrb *.synctex.gz
