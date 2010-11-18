BASEDIR = ../../
include $(BASEDIR)Make.env

CFLAGS += -fPIC

DEPS	= $(INC_MEVENT) $(INC_MEMC) $(INC_CS) $(INC_JSON) $(INC_FCGI) $(INC_SYSTEM) 

SOURCE1 = mconfig.c
SOURCES = mconfig.c mcs.c md5c.c mdata.c mdb.c mdb-priv.c mdb-pgsql.c mdb-mysql.c mdb-sqlite.c mjson.c mmemc.c mmkd.c mmisc.c mtrace.c mtypes.c mutil.c
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

include $(DEPEND)
%.o:%.c
#	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
#		echo "=== "$(CFLAGS); \
#		echo "=== "$(DEPS); fi
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
