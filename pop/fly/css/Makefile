BASEDIR = ../../../
include $(BASEDIR)Make.env

all: mbase.css mtools.css

mbase.css: local/reset.css local/base.css local/mid.css local/content.css

mtools.css: pub/ref/flowtools/css/mtabs.css pub/ref/flowtools/css/tooltip.css pub/ref/flowtools/css/expose.css \
			pub/ref/flowtools/css/overlay.css pub/ref/flowtools/css/scrollable.css \
			pub/ref/superfish/css/superfish.css pub/ref/superfish/css/superfish-vertical.css

%.css:
	$(CSSGLUE) $^ > $@


clean:
	@rm -f mbase.css mtools.css
