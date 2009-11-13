prefix		= /usr/local
bindir		= $(prefix)/bin
libdir		= $(prefix)/lib


SRC_LIB = src/sock.c src/hash.c src/handle_http.c src/cmd.c src/users.c src/channel.c src/config.c src/json.c src/json_parser.c src/plugins.c src/http.c src/extend.c src/utils.c src/hnpub.c src/ticks.c src/proxy.c src/base64.c src/pipe.c src/raw.c src/events.c src/event_kqueue.c src/event_epoll.c src/transports.c src/servers.c src/log.c src/adb.c src/smsalarm.c
SRC_BIN = src/entry.c  src/dns.c

OBJ_LIB = $(patsubst %.c, %.o, $(SRC_LIB))
OBJ_BIN = $(patsubst %.c, %.o, $(SRC_BIN))

LIB_APE = libaped.so
EXEC=bin/aped

CFLAGS=-Wall -g -D_GNU_SOURCE -minline-all-stringops -rdynamic -I/usr/local/mysql/include -I ./libs/udns-0.0.9/
LFLAGS=-ldl -lm
CC=gcc -std=c99
RM=rm -f

all: libaped aped

.c.o:
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

libaped: $(OBJ_LIB)
	$(CC) $(CFLAGS) -fPIC -shared $(OBJ_LIB) -o $(LIB_APE) $(LFLAGS) -L/usr/local/mysql/lib/mysql -lmysqlclient

aped: libaped $(SRC_BIN)
	$(CC) $(CFLAGS) $(SRC_BIN) -o $(EXEC) -L./ -laped $(LFLAGS) ./libs/udns-0.0.9/libudns.a

install: 
	install -d $(bindir)
	install -m 755 $(EXEC) $(bindir)
	install -m 755 $(LIB_APE) $(libdir)

uninstall:
	$(RM) $(bindir)/$(EXEC)
	$(RM) $(libdir)/$(LIB_APE)

clean:
	$(RM) $(LIB_APE) $(EXEC) $(OBJ_LIB) $(OBJ_BIN)
