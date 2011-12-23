BASEDIR = ../
include $(BASEDIR)Make.env

SUBDIR = mevent mnl eii

all: $(subdir)
	@$(MULTIMAKE) $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

install:
	@$(MULTIMAKE) -m install $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
