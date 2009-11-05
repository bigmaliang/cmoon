BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

all: install

install:
	@find -name '*.js' -exec $(CPWITHDIR) -p $(PATH_DOC)/js/ {} \;
	@find -name 'up.*.html' -exec $(CPWITHDIR) -p $(PATH_DOC)/uiplug/ {} \;
