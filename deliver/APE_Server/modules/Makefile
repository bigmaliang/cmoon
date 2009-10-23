include ./plateform.mk

CC=gcc -std=c99 -D_GNU_SOURCE

ifdef DARWIN_BUILD

all: modules

modules: libape-spidermonkey.c

	$(CC) -Wall -g -bundle -undefined suppress -flat_namespace -o lib/libmod_spidermonkey.so libape-spidermonkey.c -I../libs/js1.8/src/dist/include/ -L../libs/js1.8/src/ -ljs_static -lstdc++

endif
ifdef LINUX_BUILD

CFLAGS=-Wall -g -shared -fPIC -rdynamic

EVENT_BASE = ../../../../event
CS_BASE = ../../../../lib/cs
INC_EVENT = -I $(EVENT_BASE)/api -I $(EVENT_BASE)/base -I $(EVENT_BASE)/plugin -I $(CS_BASE)/include/ClearSilver
LIB_EVENT = -L $(EVENT_BASE)/api -lmevent -L $(EVENT_BASE)/daemon -lmevent_server -L/usr/local/lib -l neo_utl

all: modules

modules: libape-spidermonkey.c libape-push.c libape-mysql.c

#	$(CC) $(CFLAGS) -Wl,-soname,libmod_spidermonkey.so -o lib/libmod_spidermonkey.so libape-spidermonkey.c -I../libs/js1.8/src/dist/include/ -L../libs/js1.8/src/ -ljs_static -lstdc++
	$(CC) $(CFLAGS) -Wl,-soname,libmod_push.so -I../src -I/usr/local/mysql/include $(INC_EVENT) -o lib/libmod_push.so libape-push.c  -L../ -laped -L/usr/local/mysql/lib/mysql -lmysqlclient $(LIB_EVENT)
	$(CC) $(CFLAGS) -Wl,-soname,libmod_mysql.so -I../src -I/usr/local/mysql/include -lz -o lib/libmod_mysql.so libape-mysql.c -L../ -laped -L/usr/local/mysql/lib/mysql -lmysqlclient
endif
