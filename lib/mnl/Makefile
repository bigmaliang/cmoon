BASEDIR = ../../
include $(BASEDIR)Make.env

DEPS	= $(INC_MOON)

UNAME := $(shell uname)
CFLAGS += -fPIC -shared

SOURCE1 = mcfg.c
SOURCES = mcfg.c md5c.c mb64.c mmkd.c mtrace.c mmemc.c mjson.c mbson.c mhttp.c mdb.c mdb-pgsql.c mdb-mysql.c mdb-sqlite.c mmg.c mcs.c mstr.c mutil.c mtemplate.c mfile.c mimg.c mtimer.c
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND	= .depend

all: libmnl.so

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(DEPS) > $@

libmnl.a:$(OBJS) $(DEPEND)
	@rm -rf $@
	$(AR) rcs $@ $(OBJS)

# libmnl.so used static library's fucntions(lib_cs here)
# and, on darwin os, need to add $(LIB_CS)
# because darwin's linker won't link lib_cs to viki, mevent...
libmnl.so:$(OBJS) $(DEPEND)
ifeq ($(UNAME), Darwin)
	$(CC) $(CFLAGS) -dynamiclib -undefined suppress -flat_namespace -o $@ $(OBJS) $(LIB_STATIC)
else
	@$(CC) $(CFLAGS) -o $@ $(OBJS)
endif

include $(DEPEND)
%.o:%.c
	@if [ "$<" = "$(SOURCE1)" ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(DEPS); fi
	@echo "$(CC) -c $<"
	@$(CC) $(CFLAGS) -c $< -o $@ $(DEPS)

cflag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(DEPS)

install:
	install -m 0644 libmnl.so /usr/local/lib

uninstall:
	rm -f /usr/local/lib/libmnl.so

clean:
	rm -f $(OBJS) *.a *.so

backup:
	@$(MAKE) clean
	@$(TAR) zcvf mnl.tar.gz *
