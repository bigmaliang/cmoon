BASEDIR = ../../
include $(BASEDIR)Make.env

DEPS	= $(INC_MOON)

SOURCE1 = mconfig.c
SOURCES = mconfig.c mcs.c md5c.c mdb.c mdb-pgsql.c mdb-mysql.c mdb-sqlite.c mjson.c mmemc.c mmkd.c mmisc.c mtrace.c mtypes.c mutil.c
ifneq ($(NMYSQL), 1)
	SOURCES += fdb.c
endif
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND	= .depend

all: libmnl.a

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(DEPS) > $@

libmnl.a:$(OBJS) $(DEPEND)
	@rm -rf $@
	$(AR) rcs $@ $(OBJS)
	$(AR) d $@ $(DROPOBJS)

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
