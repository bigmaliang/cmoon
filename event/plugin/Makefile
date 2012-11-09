BASEDIR = ../../
include $(BASEDIR)Make.env

UNAME := $(shell uname)

# prefix for installing the binaries
PREFIX=/usr/local

# -rdynamic instructs the linker to add all symbols,
# not only used ones, to the dynamic symbol table.
# e.g. nm mevent_skeleton.so | grep mjson output null
# if [-rdynamic $(LIB_MOON)] don't supply
# and this will occur error on dlysm if:
# nm daemon/mevent | grep mjson output null
# at the same time
#CFLAGS += -shared -fPIC -rdynamic
CFLAGS += -shared -fPIC

CFLAGS += -I$(BASEDIR)event/daemon/ $(INC_MOON)

SOURCES = $(wildcard *.c)
LIB_PLUGIN = $(patsubst %.c, %.so, $(SOURCES))

all: $(LIB_PLUGIN)

mevent_%.so: mevent_%.c
	@echo "$(CC) -o $@"
ifeq ($(UNAME), Darwin)
	@$(CC) $(CFLAGS) -c $<
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
