BASEDIR = ../../
include $(BASEDIR)Make.env
LOCALBASE = ../
include $(LOCALBASE)Make.env

INCS	= $(INC_LPUB) $(INC_DB) $(PKG_NMDB)
LIBS	= $(LIB_LPUB) $(LIB_DB) $(PKG_MEVENT) $(PKG_MNL) $(PKG_MEMC) $(PKG_CS) $(PKG_JSON) $(PKG_FCGI) $(PKG_SYSTEM)

SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %.cgi, $(SOURCES))
DEPEND = .depend

all:$(DEPEND) preload $(BINARY)

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(INCS) > $(DEPEND)

preload:
	@$(MAKE) -C $(LPUBBASE)
	@$(MAKE) -C $(DBBASE)

include $(DEPEND)
%.cgi:%.c
	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INCS); \
		echo "=== "$(LIBS); fi
	@echo "$(CC) -o $@"
	@$(CC) $(CFLAGS) $< -o $@ $(INCS) ${LIBS}

cflag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(INCS)
	@echo "=== "$(LIBS)

clean:
	@rm -rf ${BINARY}

backup:
	@$(MAKE) clean
	@$(TAR) zcvf cgi.tar.gz *

install:all
	@mkdir -p -m 775 $(PATH_CGI)
	@strip $(BINARY)
	@cp $(BINARY) $(PATH_CGI)/
	@chmod a+x $(PATH_CGI)/$(BINARY)

distclean:
	@cd $(PATH_CGI); rm -rf $(BINARY); cd -
	@rm -rf ${BINARY}
