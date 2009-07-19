BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

all: install

install: $(BINARY)
	@find -name '*.js' -exec $(CPWITHDIR) -p $(PATH_DOC)/js/ {} \;
