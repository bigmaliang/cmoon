BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

INCS	= $(INC_MNL) $(INC_MEMC) $(INC_LPUB) $(INC_CS) $(INC_JSON) $(INC_NMDB) $(INC_QDBM) $(INC_FCGI) $(INC_SYSTEM)

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

cflag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(INCS)

clean:
	@rm -f $(OBJS) *.a *.so

help:
	@echo "make [RLS=1]"
