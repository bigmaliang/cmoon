BASEDIR = ../../
include ../../Make.env

INCS	= $(INC_HFL) $(INC_DB) $(INC_CS) $(INC_JSON) $(INC_QDBM) $(INC_FCGI) $(INC_SYSTEM)
LIBS	= $(LIB_HFL) $(LIB_DB) $(LIB_CS) $(LIB_JSON) $(LIB_QDBM) $(LIB_FCGI) $(LIB_SYSTEM)

SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %.cgi, $(SOURCES))
DEPEND = .depend

all:$(DEPEND) preload $(BINARY)

$(DEPEND): $(SOURCES)
	@touch $@
	@$(CC) $(CFLAGS) -MM $^ $(INCS) > $(DEPEND)
include $(DEPEND)

preload:
	@$(MAKE) -C $(HFLBASE)
	@$(MAKE) -C $(DBBASE)

%.cgi:%.c
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
	@mkdir -p -m 775 $(PATH_CGI)
	@strip $(BINARY)
	@cp $(BINARY) $(PATH_CGI)/
	@chmod a+x $(PATH_CGI)/$(BINARY)
clean:
	@rm -rf ${BINARY}
distclean:
	@cd $(PATH_CGI); rm -rf $(BINARY); cd -
	@rm -rf ${BINARY}
