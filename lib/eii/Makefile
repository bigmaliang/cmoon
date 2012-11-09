BASEDIR = ../../
include $(BASEDIR)Make.env

DEPS	= $(INC_MOON)

UNAME := $(shell uname)
CFLAGS += -fPIC -shared

SOURCES = ediv.c
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND	= .depend

all: libeii.so

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(DEPS) > $@

libeii.a:$(OBJS) $(DEPEND)
	@rm -rf $@
	$(AR) rcs $@ $(OBJS)

libeii.so:$(OBJS) $(DEPEND)
ifeq ($(UNAME), Darwin)
	@$(CC) $(CFLAGS) -dynamiclib -undefined suppress -flat_namespace -o $@ $(OBJS) $(LIB_STATIC)
else
	@$(CC) $(CFLAGS) -o $@ $(OBJS)
endif

include $(DEPEND)
%.o:%.c
	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(DEPS); fi
	@echo "$(CC) -c $<"
	@$(CC) $(CFLAGS) -c $< -o $@ $(DEPS)

cflag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(DEPS)

install:
	install -m 0644 libeii.so /usr/local/lib

uninstall:
	rm -f /usr/local/lib/libeii.so

clean:
	rm -f $(OBJS) *.a *.so

backup:
	@$(MAKE) clean
	@$(TAR) zcvf eii.tar.gz *
