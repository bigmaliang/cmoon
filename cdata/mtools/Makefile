BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

INCS	= $(INC_LPUB) $(INC_DB) $(INC_MNL) $(INC_MEMC) $(INC_CS) $(INC_JSON) $(INC_NMDB) $(INC_QDBM) $(INC_FCGI) $(INC_QDBM) $(INC_SYSTEM)
LIBS	= $(LIB_LPUB) $(LIB_DB) $(LIB_MNL) $(LIB_MEMC) $(LIB_CS) $(LIB_JSON) $(LIB_NMDB) $(LIB_QDBM) $(LIB_FCGI) $(LIB_NMDB) $(LIB_QDBM) $(LIB_SYSTEM)

SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %, $(SOURCES))
SCRIPTS = keeprunning.sh
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
	@strip $(BINARY)
	@cp $(BINARY) $(SCRIPTS) $(PATH_MTLS)/
	@chmod a+x $(PATH_MTLS)/$(BINARY) $(PATH_MTLS)/$(SCRIPTS)
clean:
	@rm -rf ${BINARY}
distclean:
	@cd $(PATH_MTLS); rm -rf $(BINARY); cd -
	@rm -rf ${BINARY}
