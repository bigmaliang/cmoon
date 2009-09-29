BASEDIR = ./
include $(BASEDIR)Make.env

SUBDIR = lib event cdata rock

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
