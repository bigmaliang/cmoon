BASEDIR = ../../
include $(BASEDIR)Make.env

CC = gcc
CFLAGS = -g

INCS	= -I ../api -I../base -I../daemon $(INC_CS)
LIBS	= -L ../api -lmevent -L../daemon/ -lmevent_server -levent -lpthread -lrt -ldl  $(LIB_MYSQL) $(LIB_NEOUTL_DYN)

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
