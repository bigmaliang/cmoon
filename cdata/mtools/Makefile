BASEDIR = ../../
include ../../Make.env

INCS	= $(INC_HFL) $(INC_DB) $(INC_CS) $(INC_JSON) $(INC_QDBM) $(INC_FCGI) $(INC_SYSTEM)
LIBS	= $(LIB_HFL) $(LIB_DB) $(LIB_CS) $(LIB_JSON) $(LIB_QDBM) $(LIB_FCGI) $(LIB_SYSTEM)

SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %, $(SOURCES))
SCRIPTS = keeprunning.sh
DEPEND = .depend

all:$(DEPEND) preload $(BINARY)

$(DEPEND): $(SOURCES)
	@touch $@
	@$(CC) $(CFLAGS) -MM $^ $(INCS) > $(DEPEND)
include $(DEPEND)

preload:
	@$(MAKE) -C $(HFLBASE)
	@$(MAKE) -C $(DBBASE)

%:%.c
	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INCS); \
		echo "=== "$(LIBS); fi
	@echo "$(CC) -o $@"
	@$(CC) $(CFLAGS) $< -o $@ $(INCS) ${LIBS}

flag:
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
