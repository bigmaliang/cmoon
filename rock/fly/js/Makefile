BASEDIR = ../../../
include $(BASEDIR)Make.env

.PHONY: pub
all: pub llayout.js

pub:
	@$(MAKE) -C pub

llayout.js: local/util.js local/base.js local/login.js

%.js:
	@echo -n > $@
	$(JSGLUE) $^ > $@


clean:
	@$(MAKE) -C pub clean
	@rm -f llayout.js lregist.js llimit.js
