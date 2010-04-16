BASEDIR = ./
include $(BASEDIR)Make.env

SUBDIR = event lib rock cdata

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
