CFLAGS = -D_GNU_SOURCE -std=c99 -Wall -g -shared -fPIC -rdynamic
CC=gcc
INCLUDE_PATH =  -Iphp/ -Iphp/include -Iphp/main/ -Iphp/Zend -Iphp/ZendEngine2 -Iphp/TSRM
PHP_LIB=php/libs/libphp5.a

EVENT_BASE = ../../../event
CS_BASE = ../../../lib/cs

INC_EVENT = -I $(EVENT_BASE)/api -I $(EVENT_BASE)/base -I $(EVENT_BASE)/plugin -I $(CS_BASE)/include/ClearSilver

LIB_EVENT = -L $(EVENT_BASE)/api -lmevent -L $(EVENT_BASE)/daemon -lmevent_server -L/usr/local/lib -l neo_utl

all: modules

modules: libape-chat.c libape-mysql.c libape-setcord.c libape-helloworld.c libape-push.c
	$(CC) $(CFLAGS) -Wl,-soname,libmod_setcord.so -I../src -I/usr/local/mysql/include -fPIC -shared -o lib/libmod_setcord.so libape-setcord.c -L../ -laped
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_mouse.so -o lib/libmod_mouse.so libape-mouse.c
	$(CC) $(CFLAGS) -Wl,-soname,libmod_controller.so -I../src -I/usr/local/mysql/include -fPIC -shared -o lib/libmod_controller.so libape-controller.c  -L../ -laped	
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_chat.so -I../src -fPIC -shared -o lib/libmod_chat.so libape-chat.c -L../ -laped
	$(CC) $(CFLAGS) -Wl,-soname,libmod_push.so -I../src -I/usr/local/mysql/include $(INC_EVENT) -fPIC -shared -o lib/libmod_push.so libape-push.c  -L../ -laped -L/usr/local/mysql/lib/mysql -lmysqlclient $(LIB_EVENT)
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_spidermonkey.so -o lib/libmod_spidermonkey.so libape-spidermonkey.c -I../../js/src/ -I../../js/src/Linux_All_DBG.OBJ -L../../js/src/Linux_All_DBG.OBJ/ -ljs
	$(CC) $(CFLAGS) -Wl,-soname,libmod_mysql.so -I../src -I/usr/local/mysql/include -fPIC -shared  -lz -o lib/libmod_mysql.so libape-mysql.c -L../ -laped -L/usr/local/mysql/lib/mysql -lmysqlclient
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_chat.so -o lib/libmod_chat.so libape-chat.c -L./lib -lmod_mysql -Xlinker "-R\$$ORIGIN"
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_helloworld.so -o lib/libmod_helloworld.so libape-helloworld.c
	#$(CC) -Wall -o ./objs/libape-php.o -c libape-php.c $(INCLUDE_PATH)
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_php.so -o lib/libmod_php.so objs/libape-php.o $(PHP_LIB) -ldl -lm -lcrypt -lresolv -lxml2 -lpthread
#./configure --enable-embed=static --with-zlib --enable-ftp --enable-static=zlib --disable-simplexml --disable-xmlreader --disable-xmlwriter --disable-soap


