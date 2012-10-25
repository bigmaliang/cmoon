BASEDIR = ../../
include $(BASEDIR)Make.env

DEPS	= $(INC_MOON)

SOURCE1 = mcfg.c
SOURCES = mcfg.c md5c.c mb64.c mmkd.c mtrace.c mmemc.c mjson.c mhttp.c mdb.c mdb-pgsql.c mdb-mysql.c mdb-sqlite.c mcs.c mstr.c mutil.c mtemplate.c mfile.c mimg.c
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND	= .depend

all: libmnl.a

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(DEPS) > $@

libmnl.a:$(OBJS) $(DEPEND)
	@rm -rf $@
	$(AR) rcs $@ $(OBJS)

# make clean &&  make DYN=1 libmnl.so to make shared library
libmnl.so:$(OBJS) $(DEPEND)
	@$(CC) $(CFLAGS) -o $@ $(OBJS)

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

clean:
	@rm -f $(OBJS) *.a

backup:
	@$(MAKE) clean
	@$(TAR) zcvf mnl.tar.gz *
