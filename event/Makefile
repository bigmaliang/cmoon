BASEDIR = ../
include ../Make.env

SUBDIR = daemon api conf test

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

install:
	@$(MULTIMAKE) -m install $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
