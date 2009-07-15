BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

INCS	= $(INC_MNL) $(INC_MEMC) $(INC_LPUB) $(INC_CS) $(INC_JSON) $(INC_QDBM) $(INC_FCGI) $(INC_FCGI) $(INC_SYSTEM)

SOURCES = $(wildcard *.c)
OBJS	= $(patsubst %.c, %.o, $(SOURCES))
DEPEND = .depend

all:$(DEPEND) libodb.a

$(DEPEND): $(SOURCES)
	@touch $@
	@$(CC) $(CFLAGS) -MM $^ $(INCS) > $(DEPEND)
include $(DEPEND)

libodb.a:$(OBJS)
	@rm -rf $@
	$(AR) rcs $@ $^

%.o:%.c
	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INCS); fi
	@echo "$(CC) -c $<"
	@$(CC) $(CFLAGS) -c $< -o $@ $(INCS)

flag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(INCS)

clean:
	@rm -f $(OBJS) *.a
