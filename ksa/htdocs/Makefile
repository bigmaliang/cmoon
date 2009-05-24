BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

release:
	scp -rp * $(USER_DEST)@$(SITE_DEST):$(PATH_DEST_DOC)
