BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

INCS	= $(PKG_MEVENT) $(PKG_MNL) $(PKG_MEMC) $(PKG_CS) $(PKG_JSON) $(PKG_FCGI) $(PKG_SYSTEM) $(INC_PUB)

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

