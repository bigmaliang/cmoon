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
	int ret, times, fai, suc;
	char host[64] = "127.0.0.1";
	int cmd, uin, bonus, exp;
	uin = bonus = exp = 0;
	cmd = 1001;
	
	if (argc > 1) {
		times = atoi(argv[1]);
	} else {
		printf("Usage: %s TIMES [HOST] [COMMAND] [UIN] [BONUS] [EXP]\n", argv[0]);
		return 1;
	}
	if (argc > 2) {
		strncpy(host, argv[2], sizeof(host));
	}
	if (argc > 3) {
		cmd = atoi(argv[3]);
	}
	if (argc > 4) {
		uin = atoi(argv[4]);
	}
	if (argc > 5) {
		bonus = atoi(argv[5]);
	}
	if (argc > 6) {
		exp = atoi(argv[6]);
	}

/* for change these 3 lines of code into one, sooo tired */
#if 0
	evt = mevent_init();
	if (evt == NULL) {
		printf("init error\n");
		return 1;
	}

	mevent_add_tcp_server(evt, host, 26000);
	mevent_chose_plugin(evt, "udc", 1001, FLAGS_SYNC);
#endif
	
	evt = mevent_init_plugin("udc", cmd, FLAGS_SYNC);
	if (uin != 0)
		mevent_add_u32(evt, NULL, "uin", uin);
	if (bonus != 0)
		mevent_add_u32(evt, NULL, "bonus", bonus);
	if (exp != 0)
		mevent_add_u32(evt, NULL, "exp", exp);
	
	suc = fai = 0;
	timer_start();
	int i;
	for (i = 0; i < times; i++) {
		ret = mevent_trigger(evt);
		if (PROCESS_OK(ret)) {
			printf("process success %d\n", ret);
			data_cell_dump(evt->rcvdata);
			suc++;
		} else {
			printf("process failure %d!\n", ret);
			fai++;
		}
	}
	s_elapsed = timer_stop();
	printf("%lu\n", s_elapsed);
	printf("suc %d fai %d\n", suc, fai);

	mevent_free(evt);
	return 0;
}
