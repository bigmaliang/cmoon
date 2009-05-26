BASEDIR = ../
include ../Make.env

SUBDIR = hifly db cgi mtools

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

install: $(SUBDIR)
	@$(MULTINAME) -m install $(SUBDIR)
	@cp -f config.hdf $(PATH_DOC)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
