BASEDIR = ../../
include $(BASEDIR)Make.env

DEPS	= $(INC_MOON)

SOURCES = ediv.c
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND	= .depend

all: libeii.a

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(DEPS) > $@

libeii.a:$(OBJS) $(DEPEND)
	@rm -rf $@
	$(AR) rcs $@ $(OBJS)

# make clean &&  make DYN=1 libeii.so to make shared library
libeii.so:$(OBJS) $(DEPEND)
	@$(CC) $(CFLAGS) -o $@ $(OBJS)

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

clean:
	@rm -f $(OBJS) *.a

backup:
	@$(MAKE) clean
	@$(TAR) zcvf eii.tar.gz *
