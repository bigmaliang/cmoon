BASEDIR = ../
include $(BASEDIR)Make.env
LOCALBASE = ./
include $(LOCALBASE)Make.env

SUBDIR = hifly db cgi mtools

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

install: $(SUBDIR)
	@$(MULTIMAKE) -m install $(SUBDIR)
	@cp -f config.hdf $(PATH_DOC)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
