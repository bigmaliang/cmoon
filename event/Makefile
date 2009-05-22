MAKE = make

.PHONY: api daemon test
all: api daemon test

api:
	@echo ""
	@echo "############ api ############"
	@echo "============================="
	@$(MAKE)  -C ./api ENABLE_SCTP=0 ENABLE_TIPC=0

daemon:
	@echo ""
	@echo "############ daemon ############"
	@echo "============================="
	@$(MAKE) -C ./daemon ENABLE_SCTP=0 ENABLE_TIPC=0

test:
	@echo ""
	@echo "############ test ############"
	@echo "============================="
	@$(MAKE) -C ./test


install: api daemon
	@echo ""
	@echo "############ install ############"
	@echo "================================="
	@$(MAKE) -C ./api install
	@$(MAKE) -C ./daemon install

clean:
	@echo ""
	@echo "############ clean ############"
	@echo "==============================="
	@$(MAKE) -C ./api clean
	@$(MAKE) -C ./daemon clean
	@$(MAKE) -C ./test clean
