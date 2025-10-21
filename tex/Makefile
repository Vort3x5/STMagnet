name := intro

sprawko:
	pdflatex $(name).tex
	pdflatex $(name).tex
	evince $(name).pdf &

clean:
	if pgrep evince; then pkill evince; fi
	rm -f $(name).aux $(name).log $(name).out $(name).pdf
