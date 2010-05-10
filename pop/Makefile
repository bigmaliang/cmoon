BASEDIR = ../
include ../Make.env

#SUBDIR = pub walk run pager
SUBDIR = pager

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
