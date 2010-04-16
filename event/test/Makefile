BASEDIR = ../../
include $(BASEDIR)Make.env

CC = gcc
CFLAGS = -g

INCS	= -I../daemon $(PKG_MEVENT)
LIBS	= -levent -lpthread -lrt -ldl

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
