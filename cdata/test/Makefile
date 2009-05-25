LOCALBASE = ../../
include $(LOCALBASE)Make.env

HTMLS = $(wildcard *.html)

install:
	@mkdir -p -m 775 $(PATH_DOC)
	@cp $(HTMLS) $(PATH_DOC)/
