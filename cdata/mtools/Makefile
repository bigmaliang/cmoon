BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

INCS	= $(INC_LPUB) $(INC_DB) $(PKG_NMDB) $(PKG_QDBM)
LIBS	= $(LIB_LPUB) $(LIB_DB) $(PKG_MEVENT) $(PKG_MNL) $(PKG_MEMC) $(PKG_CS) $(PKG_JSON) $(PKG_FCGI) $(PKG_SYSTEM)

SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %, $(SOURCES))
SCRIPTS = keeprunning.sh storedb.sh storedb_8.sh storedb_9.sh hb.sh
DEPEND = .depend

all:preload $(BINARY)

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(INCS) > $@

preload:
	@$(MAKE) -C $(LPUBBASE)
	@$(MAKE) -C $(DBBASE)

include $(DEPEND)
%:%.c
	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INCS); \
		echo "=== "$(LIBS); fi
	@echo "$(CC) -o $@"
	@$(CC) $(CFLAGS) $< -o $@ $(INCS) ${LIBS}

cflag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(INCS)
install:all
	@mkdir -p -m 775 $(PATH_MTLS)
	@mkdir -p -m 775 $(PATH_MTLS)/dbs/
	@strip $(BINARY)
	@cp $(BINARY) $(SCRIPTS) $(PATH_MTLS)/
	@chmod a+x $(PATH_MTLS)/$(BINARY) $(PATH_MTLS)/$(SCRIPTS)
clean:
	@rm -rf ${BINARY}
distclean:
	@cd $(PATH_MTLS); rm -rf $(BINARY); cd -
	@rm -rf ${BINARY}
