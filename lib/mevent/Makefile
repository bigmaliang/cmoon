BASEDIR = ../../
include $(BASEDIR)Make.env

# prefix for installing the binaries
PREFIX=/usr/local

UNAME := $(shell uname)
CFLAGS += -fPIC -shared

# Protocols to enable
ENABLE_TIPC = 0
ENABLE_TCP = 1
ENABLE_UDP = 1
ENABLE_SCTP = 0

CFLAGS += -DENABLE_TIPC=$(ENABLE_TIPC) \
		-DENABLE_TCP=$(ENABLE_TCP) \
		-DENABLE_UDP=$(ENABLE_UDP) \
		-DENABLE_SCTP=$(ENABLE_SCTP)


SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SOURCES))


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
	@echo "$(CC) -o $@"
	@$(CC) $(CFLAGS) -I /usr/local/include/ClearSilver -c $< -o $@

libs: libmevent.so

libmevent.so: mevent.h $(OBJS)
ifeq ($(UNAME), Darwin)
	$(CC) $(CFLAGS) -dynamiclib -undefined suppress -flat_namespace -o libmevent.so $(OBJS) $(LIB_STATIC)
else
	$(CC) $(CFLAGS) -o libmevent.so $(OBJS)
endif

libmevent.a: mevent.h $(OBJS)
	$(AR) cr libmevent.a $(OBJS)


install-lib: libs libmevent.pc
	install -d $(PREFIX)/lib
	install -m 0644 libmevent.so $(PREFIX)/lib
#	install -m 0755 libmevent.a $(PREFIX)/lib
	install -d $(PREFIX)/include
	install -m 0644 mevent.h $(PREFIX)/include
	install -m 0644 net-const.h $(PREFIX)/include
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


