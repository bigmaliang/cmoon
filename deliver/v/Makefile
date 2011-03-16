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


ALL_CFLAGS += -I./ -I /usr/local/include/ClearSilver $(INC_MEVENT) \
			  $(INC_MNL) $(INC_MEMC) $(INC_CS) $(INC_JSON) $(INC_FCGI) $(INC_SYSTEM)
LIBS = -levent -lneo_utl $(LIB_MEVENT) \
	   $(LIB_MNL) $(LIB_MEMC) $(LIB_CS) $(LIB_JSON) $(LIB_FCGI) $(LIB_SYSTEM)


BASE_SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(BASE_SOURCES))


default: all

all: v

.c.o:
	$(NICE_CC) $(ALL_CFLAGS) -fPIC -c $< -o $@

mevent_%.so: ../plugin/mevent_%.c
	$(NICE_CC) $(ALL_CFLAGS) -shared -fPIC -o $@ $<

v: $(OBJS)
	$(NICE_CC) $(ALL_CFLAGS) $(OBJS) -o v $(LIBS)


install-bin: v
	install -d $(PREFIX)/bin
	install -m 0755 v $(PREFIX)/bin/

install: install-bin

uninstall:
	rm -f $(PREFIX)/bin/v

clean: clean-build clean-prof

clean-build:
	rm -f $(OBJS) $(OBJS_MAIN) $(OBJS_HB) $(LIB_PLUGIN) v

clean-prof:
	rm -f *.bb *.bbg *.da *.gcov *.gcda *.gcno gmon.out

.PHONY: default all \
	install-bin install-man install-lib install uninstall \
	clean-build clean-prof clean

