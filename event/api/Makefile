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

CFLAGS += -std=c99 -pedantic -Wall -fno-strict-aliasing
ALL_CFLAGS = -D_XOPEN_SOURCE=500 -fPIC $(CFLAGS)
ALL_CFLAGS += -DENABLE_TIPC=$(ENABLE_TIPC) \
		-DENABLE_TCP=$(ENABLE_TCP) \
		-DENABLE_UDP=$(ENABLE_UDP) \
		-DENABLE_SCTP=$(ENABLE_SCTP)

ifdef DEBUG
ALL_CFLAGS += -g
else
ALL_CFLAGS += -O3
endif

ifdef PROFILE
ALL_CFLAGS += -g -pg -fprofile-arcs -ftest-coverage
endif


ALL_CFLAGS += -I../base -I../daemon $(INC_CS)


BASE_SOURCES = $(wildcard ../base/*.c)

OBJS = libmevent.o tcp.o tipc.o udp.o sctp.o
OBJS += $(patsubst %.c, %.o, $(BASE_SOURCES))



default: all

all: libs libmevent.pc

mevent.h: mevent.skel.h
	@echo "generating mevent.h"
	@cat mevent.skel.h | \
		sed 's/++CONFIG_ENABLE_TIPC++/$(ENABLE_TIPC)/g' | \
		sed 's/++CONFIG_ENABLE_TCP++/$(ENABLE_TCP)/g' | \
		sed 's/++CONFIG_ENABLE_UDP++/$(ENABLE_UDP)/g' | \
		sed 's/++CONFIG_ENABLE_SCTP++/$(ENABLE_SCTP)/g' \
		> mevent.h

libmevent.pc: libmevent.skel.pc
	@echo "generating libmevent.pc"
	@cat libmevent.skel.pc | \
		sed 's@++PREFIX++@$(PREFIX)@g' \
		> libmevent.pc

.c.o:
	$(NICE_CC) $(ALL_CFLAGS) -fPIC -c $< -o $@

#libs: libmevent.a libmevent.so
libs: libmevent.a

libmevent.so: mevent.h $(OBJS)
	$(NICE_CC) $(ALL_CFLAGS) -shared -fPIC $(OBJS) -o libmevent.so

libmevent.a: mevent.h $(OBJS)
#	for php use, only linke with libmevent
	@cp $(NEOUTL) libmevent.a
	$(AR) cr libmevent.a $(OBJS)


install-lib: libs libmevent.pc
	install -d $(PREFIX)/lib
#	install -m 0755 libmevent.so $(PREFIX)/lib
	install -m 0755 libmevent.a $(PREFIX)/lib
	install -d $(PREFIX)/include
	install -m 0644 mevent.h $(PREFIX)/include
	install -m 0644 ../base/net-const.h $(PREFIX)/include
	install -d $(PREFIX)/lib/pkgconfig
	install -m 644 libmevent.pc $(PREFIX)/lib/pkgconfig
	@echo
	@echo "Please run ldconfig to update your library cache"
	@echo

install-man:
	install -d $(PREFIX)/man/man3
	install -m 0644 libmevent.3 $(PREFIX)/man/man3/

install: install-lib install-man


clean:
	rm -f mevent.h libmevent.pc $(OBJS) libmevent.so libmevent.a
	rm -f *.bb *.bbg *.da *.gcov *.gcda *.gcno gmon.out

.PHONY: default all libs install-lib install-man install clean


