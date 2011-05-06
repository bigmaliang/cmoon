BASEDIR = ../../
include $(BASEDIR)Make.env

# prefix for installing the binaries
PREFIX=/usr/local

# cc command line
ifneq ($(V), 1)
	NICE_CC = @echo "  CC  $@"; $(CC)
else
	NICE_CC = $(CC)
endif

ALL_CFLAGS = $(CFLAGS)
ALL_CFLAGS += -rdynamic

ifdef PROFILE
ALL_CFLAGS += -g -pg -ftest-coverage -fprofile-generate
endif

ifdef PROFILE_USE
ALL_CFLAGS += -fprofile-use
endif

ALL_CFLAGS += -I../daemon/ -I../../deliver/v/ $(INC_MEVENT) $(INC_MNL) $(INC_MEMC) $(INC_CS) $(INC_JSON) $(INC_FCGI) $(INC_SYSTEM)

SOURCES = $(wildcard *.c)
LIB_PLUGIN = $(patsubst %.c, %.so, $(SOURCES))

all: $(LIB_PLUGIN)

mevent_%.so: mevent_%.c
	$(NICE_CC) $(ALL_CFLAGS) -shared -fPIC -o $@ $<

install:
	install -d $(PREFIX)/lib
	install -m 0644 $(LIB_PLUGIN) $(PREFIX)/lib/
	install -m 0644 QQWry.Dat $(PREFIX)/lib/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB_PLUGIN)

clean:
	rm -f $(LIB_PLUGIN)
