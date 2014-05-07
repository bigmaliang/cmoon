BASEDIR = ../../
include $(BASEDIR)Make.env

UNAME := $(shell uname)

# prefix for installing the binaries
PREFIX=/usr/local

CFLAGS += -shared -fPIC

CFLAGS += -I$(BASEDIR)event/daemon/ $(INC_MOON) -Iprivate

SOURCES = $(wildcard *.c)
LIB_PLUGIN = $(patsubst %.c, %.so, $(SOURCES))
OBJ_PLUGIN = $(patsubst %.c, %.o, $(SOURCES))

all: $(LIB_PLUGIN)

mevent_%.so: private/%_internal.c private/%_timer.c mevent_%.c
	@echo "$(CC) -o $@"
ifeq ($(UNAME), Darwin)
	@$(CC) $(CFLAGS) -c $< -o $(patsubst %.c, %.o, $<)
	@$(CC) -dynamiclib -undefined suppress -flat_namespace $(patsubst %.c, %.o, $<) -o $@
else
	@$(CC) $(CFLAGS) -o $@ $<
endif

install:
	install -d $(PREFIX)/lib
	install -m 0644 $(LIB_PLUGIN) $(PREFIX)/lib/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB_PLUGIN)

clean:
	rm -f $(LIB_PLUGIN)
	rm -f $(OBJ_PLUGIN)
