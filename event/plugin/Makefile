BASEDIR = ../../
include $(BASEDIR)Make.env

# prefix for installing the binaries
PREFIX=/usr/local

CFLAGS += -shared -fPIC -rdynamic

CFLAGS += -I$(BASEDIR)event/daemon/ $(INC_MOON)

SOURCES = $(wildcard *.c)
LIB_PLUGIN = $(patsubst %.c, %.so, $(SOURCES))

all: $(LIB_PLUGIN)

mevent_%.so: mevent_%.c
	@echo "$(CC) -o $@"
	@$(CC) $(CFLAGS) -o $@ $<

install:
	install -d $(PREFIX)/lib
	install -m 0644 $(LIB_PLUGIN) $(PREFIX)/lib/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB_PLUGIN)

clean:
	rm -f $(LIB_PLUGIN)
