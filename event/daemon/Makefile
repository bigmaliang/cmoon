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

# Protocols to enable
ENABLE_TIPC = 0
ENABLE_TCP = 1
ENABLE_UDP = 1
ENABLE_SCTP = 0

ALL_CFLAGS = $(CFLAGS)
ALL_CFLAGS += -rdynamic
ALL_CFLAGS += -DENABLE_TIPC=$(ENABLE_TIPC) \
		-DENABLE_TCP=$(ENABLE_TCP) \
		-DENABLE_UDP=$(ENABLE_UDP) \
		-DENABLE_SCTP=$(ENABLE_SCTP)

ifdef PROFILE
ALL_CFLAGS += -g -pg -ftest-coverage -fprofile-generate
endif

ifdef PROFILE_USE
ALL_CFLAGS += -fprofile-use
endif


ALL_CFLAGS += -I./ $(INC_MEVENT) $(INC_MNL) $(INC_MEMC) $(INC_CS) $(INC_JSON) $(INC_FCGI) $(INC_SYSTEM)
LIBS = -levent -lpthread -lrt -ldl $(LIB_MEVENT) $(LIB_MNL) $(LIB_MEMC) $(LIB_CS) $(LIB_JSON) $(LIB_FCGI) $(LIB_SYSTEM)


BASE_SOURCES = $(wildcard ../base/*.c)
PLUGIN_SOURCES = $(wildcard ../plugin/*.c)

OBJS = mevent.o queue.o log.o net.o parse.o reply.o stats.o config.o common.o syscmd.o smsalarm.o
OBJS += $(patsubst %.c, %.o, $(BASE_SOURCES))
OBJS_MAIN = main.o
OBJS_HB = hb.o

ifeq ($(ENABLE_TIPC), 1)
	OBJS += tipc.o
else
	OBJS += tipc-stub.o
endif

ifeq ($(ENABLE_TCP), 1)
	OBJS += tcp.o
else
	OBJS += tcp-stub.o
endif

ifeq ($(ENABLE_UDP), 1)
	OBJS += udp.o
else
	OBJS += udp-stub.o
endif

ifeq ($(ENABLE_SCTP), 1)
	OBJS += sctp.o
else
	OBJS += sctp-stub.o
endif


LIB_PLUGIN = $(patsubst %.c, %.so, $(PLUGIN_SOURCES))
default: all

all: $(LIB_PLUGIN) mevent hb

.c.o:
	$(NICE_CC) $(ALL_CFLAGS) -fPIC -c $< -o $@

mevent_%.so: ../plugin/mevent_%.c
	$(NICE_CC) $(ALL_CFLAGS) -shared -fPIC -o $@ $<

mevent: $(OBJS_MAIN) $(OBJS)
	$(NICE_CC) $(ALL_CFLAGS) $(OBJS_MAIN) $(OBJS) -o mevent $(LIBS)

hb: $(OBJS_HB) $(OBJS)
	$(NICE_CC) $(ALL_CFLAGS) $(OBJS_HB) $(OBJS) -o hb $(LIBS)



install-bin: mevent
	install -d $(PREFIX)/bin
	install -m 0755 mevent $(PREFIX)/bin/
	install -m 0755 hb $(PREFIX)/bin/

install-man:
	install -d $(PREFIX)/man/man1
	install -m 0644 mevent.1 $(PREFIX)/man/man1/

install-lib:
	install -d $(PREFIX)/lib
	install -m 0644 $(LIB_PLUGIN) $(PREFIX)/lib/

install: install-bin install-man install-lib

uninstall:
	rm -f $(PREFIX)/bin/mevent
	rm -f $(PREFIX)/man/man1/mevent.1
	rm -f $(PREFIX)/lib/$(LIB_PLUGIN)

clean: clean-build clean-prof

clean-build:
	rm -f $(OBJS) $(OBJS_MAIN) $(OBJS_HB) $(LIB_PLUGIN) mevent

clean-prof:
	rm -f *.bb *.bbg *.da *.gcov *.gcda *.gcno gmon.out

.PHONY: default all \
	install-bin install-man install-lib install uninstall \
	clean-build clean-prof clean

