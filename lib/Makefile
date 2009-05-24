BASEDIR = ../
include $(BASEDIR)Make.env

SUBDIR = mnl js

all: $(subdir)
	@$(MULTIMAKE) $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
