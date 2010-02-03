BASEDIR = ../
include $(BASEDIR)Make.env
LOCALBASE = ./
include $(LOCALBASE)Make.env

SUBDIR = hifly db cgi mtools

all: $(SUBDIR)
	@$(MULTIMAKE) $(SUBDIR)

install: $(SUBDIR)
	@$(MULTIMAKE) -m install cgi mtools
	@mkdir -p -m 775 $(PATH_RPC)
	@cp -f config.hdf $(PATH_RPC)
	@cp -f lighttpd.conf $(PATH_RPC)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
