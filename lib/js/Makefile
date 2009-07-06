BASEDIR = ../../
include $(BASEDIR)Make.env

all: mbase.js mbox.js mform.js mflbu.js mseekatt.js msuperfish.js mlist.js mtable.js mpagenav.js

mbase.js: ref/jquery-1.2.6.js  ref/jquery.cookie.js  own/jquery.mnmisc.js 

mbox.js: ref/facebox/facebox.js

mform.js: ref/jquery.md5.js  ref/jquery.formval.js  ref/jquery.form.js

mflbu.js: ref/jquery.flobu.js

mseekatt.js: ref/jquery.seekAttention.js

msuperfish.js: ref/superfish/js/hoverIntent.js ref/superfish/js/superfish.js ref/superfish/js/supersubs.js \
			  own/jquery.mnsuperfish.js

mlist.js: own/jquery.mnlist.js

mtable.js: own/jquery.mntable.js

mpagenav.js: own/jquery.mnpagenav.js

%.js:
	@echo -n > $@
	$(JSGLUE) $^ > $@


clean:
	@rm -f mbase.js mbox.js mform.js mflbu.js mseekatt.js msuperfish.js mlist.js mtable.js mpagenav.js
