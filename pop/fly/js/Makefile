BASEDIR = ../../../
include $(BASEDIR)Make.env

.PHONY: pub
all: pub

pub:
	@$(MAKE) -C pub

%.js:
	@echo -n > $@
	$(JSGLUE) $^ > $@


clean:
	@$(MAKE) -C pub clean
	@rm -f llayout.js
