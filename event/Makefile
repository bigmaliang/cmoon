BASEDIR = ../
include ../Make.env

SUBDIR = api daemon conf test

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

install:
	@$(MULTIMAKE) -m install $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
