BASEDIR = ../
include $(BASEDIR)Make.env

SUBDIR = mevent mnl

all: $(subdir)
	@$(MULTIMAKE) $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
