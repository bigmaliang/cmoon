BASEDIR = ./
include $(BASEDIR)Make.env

SUBDIR = lib rock event cdata

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
