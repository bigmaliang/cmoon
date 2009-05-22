CC = gcc
AR = ar
MAKE = make
MKDIR = --no-print-directory
RANLIB = ranlib

ifeq ($(RLS), 1)
CFLAGS = -Wall -O3 -DRELEASE
STRIP = strip
else
CFLAGS = -g -Wall -DDEBUG_HDF
STRIP = touch
endif

BASEDIR = ../
HFLBASE = $(BASEDIR)/hifly
CSBASE = $(BASEDIR)/cs
JSONBASE = $(BASEDIR)/jsonc
FCGIBASE = $(BASEDIR)/fcgi

INC_HFL = -I$(HFLBASE)
INC_CS = -I$(CSBASE)/include/ClearSilver
INC_JSON = -I$(JSONBASE)/include/json
INC_FCGI = -I$(FCGIBASE)/include
INC_SYSTEM = -I/usr/local/mysql/include/mysql

INCS	= $(INC_HFL) $(INC_CS) $(INC_JSON) $(INC_FCGI) $(INC_SYSTEM)

SOURCES = $(wildcard *.c)
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND	= .depend

all:$(DEPEND) libhifly.a

$(DEPEND): $(SOURCES)
	@touch $@
	@$(CC) $(CFLAGS) -MM $^ $(INCS) > $(DEPEND)
include $(DEPEND)

libhifly.a:$(OBJS)
	@rm -rf $@
	$(AR) rcs $@ $^
	$(AR) d $@ $(DROPOBJS)

libhifly.so:$(OBJS)
	@rm -rf $@
	@$(CC) -shared -fPIC $(OBJS) -o $@

%.o:%.c
	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INCS); fi
	@echo "$(CC) -c $<"
	@$(CC) $(CFLAGS) -c -fPIC $< -o $@ $(INCS)

flag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(INCS)

clean:
	@rm -f $(OBJS) *.a *.so

help:
	@echo "make [RLS=1]"
