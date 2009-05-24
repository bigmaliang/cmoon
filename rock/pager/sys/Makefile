BASEDIR = ../../../
include $(BASEDIR)Make.env
LOCALBASE = ../../
include $(LOCALBASE)Make.env

INCS	= $(INC_MNL) $(INC_MEMC) $(INC_CS) $(INC_JSON) $(INC_PUB)
LIBS	= $(LIB_MNL) $(LIB_MEMC) $(LIB_CS) $(LIB_JSON) $(LIB_PUB) $(LIB_SYSTEM)

SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %, $(SOURCES))
DEPEND = .depend

all: preload $(BINARY)

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

flag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(INCS)
install: $(BINARY)
	@$(PEREXEC) $^
clean:
	@rm -rf ${BINARY}
#distclean:
#	@rm -rf $(PATH_FRT_MTLS)/*
#	@rm -rf ${BINARY}
