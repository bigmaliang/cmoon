#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mevent.h"
#include "timer.h"

int main(int argc, char *argv[])
{
	unsigned long s_elapsed;
	mevent_t *evt;
	int ret, times, busy, err, fai, suc;
	uint32_t errcode;
	char host[64] = "127.0.0.1", uin[64] = "100";
	
	if (argc > 1) {
		times = atoi(argv[1]);
	} else {
		printf("Usage: %s TIMES [HOST] [UIN]\n", argv[0]);
		return 1;
	}
	if (argc > 2) {
		strncpy(host, argv[2], sizeof(host));
	}
	if (argc > 3) {
		strncpy(uin, argv[3], sizeof(uin));
	}

	evt = mevent_init();
	if (evt == NULL) {
		printf("init error\n");
		return 1;
	}

	mevent_add_tcp_server(evt, host, 26000);
	mevent_chose_plugin(evt, "uic", 1001, FLAGS_SYNC);
	mevent_add_str(evt, NULL, "uin", uin);

	suc = fai = err = busy = 0;
	timer_start();
	int i;
	for (i = 0; i < times; i++) {
		ret = mevent_trigger(evt, &errcode);
		if (ret == REP_OK) {
			data_cell_dump(evt->rcvdata);
			suc++;
		} else if (ret == REP_ERR) {
			if (errcode == ERR_BUSY) {
				printf("process busy %d!\n", errcode);
				busy++;
			} else {
				printf("process error %d!\n", errcode);
				err++;
			}
		} else {
			printf("process failure %d!\n", ret);
			fai++;
		}
	}
	s_elapsed = timer_stop();
	printf("%lu\n", s_elapsed);
	printf("suc %d fai %d err %d busy %d\n", suc, fai, err, busy);

	mevent_free(evt);
	return 0;
}
