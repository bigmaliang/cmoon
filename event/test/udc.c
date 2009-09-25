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
	int cmd, uin, bonus, exp;
	uin = bonus = exp = 0;
	cmd = 1001;
	
	if (argc > 1) {
		times = atoi(argv[1]);
	} else {
		printf("Usage: %s TIMES [COMMAND] [UIN] [BONUS]/[EXP]\n", argv[0]);
		return 1;
	}
	if (argc > 2) {
		cmd = atoi(argv[2]);
	}
	if (argc > 3) {
		uin = atoi(argv[3]);
	}
	if (argc > 4) {
		bonus = exp = atoi(argv[4]);
	}

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
