BASEDIR = ../
include $(BASEDIR)Make.env

SUBDIR = daemon plugin conf test

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

install:
	@$(MULTIMAKE) -m install $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
