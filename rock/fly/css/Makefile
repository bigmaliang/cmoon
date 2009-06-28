BASEDIR = ../../../
include $(BASEDIR)Make.env

all: mbase.css mbox.css msuperfish.css mlist.css mtable.css

mbase.css: local/base.css local/mid.css local/content.css

mbox.css: pub/ref/facebox/facebox.css

msuperfish.css: pub/ref/superfish/css/superfish.css pub/ref/superfish/css/superfish-vertical.css

mlist.css: local/list.css

mtable.css: local/table.css

%.css:
	$(CSSGLUE) $^ > $@


clean:
	@rm -f mbase.css mbox.css msuperfish.css mlist.css mtable.css
