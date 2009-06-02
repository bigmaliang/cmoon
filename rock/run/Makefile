BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

INCS	= $(INC_PUB) $(INC_ROBOT) $(INC_MNL) $(INC_MEMC) $(INC_CS) $(INC_JSON) $(INC_NMDB) $(INC_QDBM) $(INC_FCGI) $(INC_SYSTEM)
LIBS	= $(LIB_PUB) $(LIB_ROBOT) $(LIB_MNL) $(LIB_MEMC) $(LIB_CS) $(LIB_JSON) $(LIB_NMDB) $(LIB_QDBM) $(LIB_FCGI) $(LIB_SYSTEM) $(LIB_PUB)

SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %, $(SOURCES))
DEPEND = .depend

.PHONY: sys
all: preload $(BINARY) sys

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(INCS) > $@

preload:
	@$(MAKE) -C $(MNLBASE)
	@$(MAKE) -C $(PUBBASE)
	@$(MAKE) -C $(ROBOTBASE)

include $(DEPEND)
%:%.c
	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INCS); \
		echo "=== "$(LIBS); fi
	@echo "$(CC) -o $@"
	@$(CC) $(CFLAGS) $< -o $@ $(INCS) ${LIBS}

sys:
	@$(MAKE) -C sys

cflag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(INCS)
install:all
	@mkdir -p -m 775 $(PATH_CGI)
	@strip $(BINARY)
	@cp $(BINARY) $(PATH_CGI)/
	@chmod a+x $(PATH_CGI)/$(BINARY)
	@$(MAKE) -C sys install
clean:
	@rm -rf ${BINARY}
	@$(MAKE) -C sys clean
distclean:
	@rm -rf $(PATH_CGI)/*
	@rm -rf ${BINARY}
	@$(MAKE) -C sys distclean
