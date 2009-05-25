BASEDIR = ../../
include $(BASEDIR)Make.env

ifeq ($(NPG), 1)
DROPMACRO = -DDROP_PG
DROPOBJS = mdb-pgsql.o
endif

CFLAGS += -fPIC

INCS	= $(INC_MNL) $(INC_MEMC) $(INC_CS) $(INC_JSON) $(INC_FCGI) $(INC_NMDB) $(INC_QDBM) $(INC_SYSTEM) 

SOURCES = $(wildcard *.c)
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND	= .depend

all: libmnl.a

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(INCS) > $@

libmnl.a:$(OBJS) $(DEPEND)
	@rm -rf $@
	$(AR) rcs $@ $(OBJS)
	$(AR) d $@ $(DROPOBJS)

include $(DEPEND)
%.o:%.c
	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INCS); fi
	@echo "$(CC) -c $<"
	@$(CC) $(CFLAGS) -c $< -o $@ $(INCS)

cflag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(INCS)

clean:
	@rm -f $(OBJS) *.a

backup:
	@$(MAKE) clean
	@$(TAR) zcvf mnl.tar.gz *
