BASEDIR = ../../
include $(BASEDIR)Make.env

INCS	= -I../daemon $(INC_MOON)
LIBS	= -levent $(LIB_MOON)

SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %, $(SOURCES))

all: $(BINARY)

%:%.c
	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INCS); \
		echo "=== "$(LIBS); fi
	@echo "$(CC) -o $@"
	@$(CC) $(CFLAGS) $< -o $@ $(INCS) ${LIBS}

install:

flag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(INCS)
	@echo "=== "$(LIBS)

clean:
	@rm -rf ${BINARY}
