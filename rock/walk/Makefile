BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

INCS	= $(INC_MNL) $(INC_MEMC) $(INC_CS) $(INC_JSON) $(INC_NMDB) $(INC_QDBM) $(INC_FCGI) $(INC_PUB) $(INC_SYSTEM)

SOURCES = $(wildcard *.c)
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND  = .depend

all: libwalk.a

depend: $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(INCS) > $@

libwalk.a:$(OBJS) $(DEPEND)
	@rm -rf $@
	$(AR) rcs $@ $(OBJS)

libwalk.so:$(OBJS) $(DEPEND)
	@$(CC) $(CFLAGS) -shared -o $@ $(OBJS)

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
	@$(TAR) zcvf walk.tar.gz *

