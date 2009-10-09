BASEDIR = ../../
include $(BASEDIR)Make.env

all: mbase.js mbox.js mflowtools.js mform.js  mseekatt.js msuperfish.js mloadify.js mlist.js mtable.js mpagenav.js minputval.js

mbase.js: ref/jquery-1.2.6.js  ref/jquery.cookie.js  own/jquery.mnmisc.js 

mbox.js: ref/facebox/facebox.js

mflowtools.js: ref/flowtools/js/tools.tabs.js ref/flowtools/js/tools.tooltip.js \
		ref/flowtools/js/tools.overlay.js ref/flowtools/js/tools.expose.js ref/flowtools/js/tools.scrollable.js

mform.js: ref/jquery.md5.js  ref/jquery.formval.js  ref/jquery.form.js

mseekatt.js: ref/jquery.seekAttention.js

msuperfish.js: ref/superfish/js/hoverIntent.js ref/superfish/js/superfish.js ref/superfish/js/supersubs.js \
			  own/jquery.mnsuperfish.js

mloadify.js: ref/ajaxupload.js

mlist.js: own/jquery.mnlist.js

mtable.js: own/jquery.mntable.js

mpagenav.js: own/jquery.mnpagenav.js

minputval.js: own/jquery.inputval.js

%.js:
	@echo -n > $@
	$(JSGLUE) $^ > $@


clean:
	@rm -f mbase.js mbox.js mflowtools.js mform.js mseekatt.js msuperfish.js mloadify.js mlist.js mtable.js mpagenav.js minputval.js
