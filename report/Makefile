PAPER = hola 
TEX = $(wildcard *.tex)
BIB = references.bib
FIGS = $(wildcard figures/*.pdf figures/*.png graphs/*.tex graphs/*.pdf graphs/*.png)

.PHONY: all clean

$(PAPER).pdf: $(TEX) $(BIB) $(FIGS)
	echo $(FIGS)
	pdflatex $(PAPER)
	bibtex $(PAPER)
	pdflatex $(PAPER)
	pdflatex $(PAPER)

clean:
	rm -f *.aux *.bbl *.blg *.log *.out $(PAPER).pdf

