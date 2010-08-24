#xxxx
BASEDIR = ./
include $(BASEDIR)Make.env

SUBDIR = lib rock event ksa pop

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
