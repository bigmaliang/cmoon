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


ALL_CFLAGS += -I../daemon -I../base $(INC_CS) $(INC_MYSQL)
LIBS = -levent -lpthread -lrt -ldl  $(LIB_MYSQL) $(LIB_NEOUTL_DYN)


BASE_SOURCES = $(wildcard ../base/*.c)
PLUGIN_SOURCES = $(wildcard ../plugin/*.c)

OBJS = mevent.o queue.o log.o dtrace.o net.o parse.o reply.o stats.o config.o common.o cache.o fdb.o syscmd.o
OBJS += $(patsubst %.c, %.o, $(BASE_SOURCES))
OBJS_BIN = main.o

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


LIB_SERVER = libmevent_server.so
LIB_PLUGIN = $(patsubst %.c, %.so, $(PLUGIN_SOURCES))
default: all

all: $(LIB_SERVER) $(LIB_PLUGIN) mevent install-lib

.c.o:
	$(NICE_CC) $(ALL_CFLAGS) -fPIC -c $< -o $@

$(LIB_SERVER): $(OBJS)
	$(NICE_CC) $(ALL_CFLAGS) -shared -fPIC -o $@ $(OBJS)

mevent_%.so: ../plugin/mevent_%.c $(LIB_SERVER)
	$(NICE_CC) $(ALL_CFLAGS) -shared -fPIC -o $@ $< -L./ -lmevent_server

mevent: $(OBJS_BIN) $(LIB_SERVER)
	$(NICE_CC) $(ALL_CFLAGS) $(OBJS_BIN) -o mevent $(LIBS) -L./ -lmevent_server


install-bin: mevent
	sudo install -d $(PREFIX)/bin
	sudo install -m 0755 mevent $(PREFIX)/bin/

install-man:
	sudo install -d $(PREFIX)/man/man1
	sudo install -m 0644 mevent.1 $(PREFIX)/man/man1/

install-lib:
	sudo install -d $(PREFIX)/lib
	sudo install -m 0644 $(LIB_SERVER) $(PREFIX)/lib/
	sudo install -m 0644 $(LIB_PLUGIN) $(PREFIX)/lib/

install: install-bin install-man install-lib

uninstall:
	sudo rm -f $(PREFIX)/bin/mevent
	sudo rm -f $(PREFIX)/man/man1/mevent.1
	sudo rm -f $(PREFIX)/lib/$(LIB_SERVER)
	sudo rm -f $(PREFIX)/lib/$(LIB_PLUGIN)

clean: clean-build clean-prof

clean-build:
	sudo rm -f $(OBJS) $(OBJS_BIN) $(LIB_PLUGIN) $(LIB_SERVER) mevent

clean-prof:
	sudo rm -f *.bb *.bbg *.da *.gcov *.gcda *.gcno gmon.out

.PHONY: default all \
	install-bin install-man install-lib install uninstall \
	clean-build clean-prof clean

