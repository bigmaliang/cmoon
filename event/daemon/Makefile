BASEDIR = ../../
include $(BASEDIR)Make.env

# prefix for installing the binaries
PREFIX=/usr/local

# Protocols to enable
ENABLE_TIPC = 0
ENABLE_TCP = 1
ENABLE_UDP = 1
ENABLE_SCTP = 0

# -rdynamic instructs the linker to add all symbols,
# not only used ones, to the dynamic symbol table.
# e.g. if without -rdynamic, ./mevent will output symbol lookup error on startup (linux)
# because mevent_*.so used wlog(), reply_trigger()..., and mevent don't call them
CFLAGS += -rdynamic
CFLAGS += -DENABLE_TIPC=$(ENABLE_TIPC) \
		-DENABLE_TCP=$(ENABLE_TCP) \
		-DENABLE_UDP=$(ENABLE_UDP) \
		-DENABLE_SCTP=$(ENABLE_SCTP)


CFLAGS += $(INC_MOON)
LIBS = -levent $(LIB_MOON)


OBJS = mevent.o queue.o log.o net.o parse.o reply.o stats.o config.o common.o syscmd.o smsalarm.o
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


default: all

all: mevent hb

.c.o:
	@echo "$(CC) -o $@"
	@$(CC) $(CFLAGS) -c $< -o $@

mevent: $(OBJS_MAIN) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS_MAIN) $(OBJS) -o mevent $(LIBS)

hb: $(OBJS_HB) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS_HB) $(OBJS) -o hb $(LIBS)



install-bin: mevent hb
	install -d $(PREFIX)/bin
	install -m 0755 mevent $(PREFIX)/bin/
	install -m 0755 hb $(PREFIX)/bin/

install-man:
	install -d $(PREFIX)/man/man1
	install -m 0644 mevent.1 $(PREFIX)/man/man1/

install: install-bin install-man

uninstall:
	rm -f $(PREFIX)/bin/mevent
	rm -f $(PREFIX)/man/man1/mevent.1

clean: clean-build clean-prof

clean-build:
	rm -f $(OBJS) $(OBJS_MAIN) $(OBJS_HB) mevent hb

clean-prof:
	rm -f *.bb *.bbg *.da *.gcov *.gcda *.gcno gmon.out

.PHONY: default all \
	install-bin install-man install uninstall \
	clean-build clean-prof clean

