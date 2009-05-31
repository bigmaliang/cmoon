#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mevent.h"
#include "timer.h"

int main(int argc, char *argv[])
{
	unsigned long s_elapsed;
	int times, suc, fai, err, busy;
	char host[64], sql[1024];
	uint32_t errcode;
	int ret;

	if (argc > 1) {
		times = atoi(argv[1]);
	} else {
		printf("Usage: %s TIMES [HOST] [SQL]\n", argv[0]);
		return 1;
	}
	if (argc > 2) {
		strncpy(host, argv[2], sizeof(host));
	} else {
		strcpy(host, "127.0.0.1");
	}
	if (argc > 3) {
		strncpy(sql, argv[3], sizeof(sql));
	}

	mevent_t *evt = mevent_init();
	if (evt == NULL) {
		printf("init error\n");
		return 1;
	}

	mevent_add_udp_server(evt, host, 26010);
#if 1
	mevent_chose_plugin(evt, "db_community", REQ_CMD_SET, FLAGS_NONE);
	mevent_add_array(evt, NULL, "sqls");
	if (!strcmp(sql, ""))
		mevent_add_str(evt, "sqls", "0", "UPDATE myvideo.album SET title='没有杀毒' WHERE aid=11;");
	else
		mevent_add_str(evt, "sqls", "0", sql);
#endif
	
	int i;
	suc = fai = err = busy = 0;
	timer_start();
	for (i = 0; i < times; i++) {
#if 0
		mevent_chose_plugin(evt, "db_community", REQ_CMD_SET, FLAGS_NONE);
		mevent_add_array(evt, NULL, "sqls");
		sprintf(sql, "INSERT INTO eventcenter.events_3day(etype, fromuid, msg, eventtime) "
				" VALUES (1, 39, '%d', '%lu');", i, time(NULL));
		mevent_add_str(evt, "sqls", "0", sql);
#endif
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
		} else
			fai++;
	}
	s_elapsed = timer_stop();
	printf("%lu\n", s_elapsed);
	printf("suc %d fai %d err %d busy %d\n", suc, fai, err, busy);
	
	mevent_free(evt);
	return 0;
}
