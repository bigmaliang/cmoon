BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

INCS	= $(INC_PUB) $(INC_ROBOT) $(INC_MEVENT) $(INC_MNL) $(INC_MEMC) $(INC_CS) $(INC_JSON) $(INC_FCGI) $(INC_SYSTEM)
LIBS	= $(LIB_PUB) $(LIB_ROBOT) $(LIB_MEVENT) $(LIB_MNL) $(LIB_MEMC) $(LIB_CS) $(LIB_JSON) $(LIB_FCGI) $(LIB_SYSTEM)

SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %, $(SOURCES))
DEPEND = .depend

all: $(BINARY)

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(INCS) > $@

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
	@cp $(BINARY) $(PATH_MTLS)/
	@chmod a+x $(PATH_MTLS)/$(BINARY)
clean:
	@rm -rf ${BINARY}
distclean:
	@rm -rf $(PATH_MTLS)/*
	@rm -rf ${BINARY}
