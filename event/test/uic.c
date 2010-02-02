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
	int cmd, uin, fuin, gid;
	char xxx[1024], yyy[1024];
	int intx, inty;
	uin = fuin = gid = 0;
	cmd = 1001;
	intx = inty = 0;
	
	if (argc > 1) {
		times = atoi(argv[1]);
	} else {
		printf("Usage: %s TIMES [COMMAND] [UIN] [FUIN] [GID] [xxx] [yyy]\n", argv[0]);
		return 1;
	}
	if (argc > 2) {
		cmd = atoi(argv[2]);
	}
	if (argc > 3) {
		uin = atoi(argv[3]);
	}
	if (argc > 4) {
		fuin = atoi(argv[4]);
	}
	if (argc > 5) {
		gid = atoi(argv[5]);
	}
	if (argc > 6) {
		strncpy(xxx, argv[6], sizeof(xxx));
		intx = atoi(argv[6]);
	}
	if (argc > 7) {
		strncpy(yyy, argv[7], sizeof(yyy));
		inty = atoi(argv[7]);
	}

	evt = mevent_init_plugin("uic", cmd, FLAGS_SYNC);
	if (uin != 0)
		mevent_add_u32(evt, NULL, "uin", uin);
	if (fuin != 0)
		mevent_add_u32(evt, NULL, "blackuin", fuin);
	if (gid != 0)
		mevent_add_u32(evt, NULL, "blacktype", gid);
	//mevent_add_str(evt, NULL, "incept", incept);
	mevent_add_u32(evt, NULL, "fkqstat", intx);
	mevent_add_u32(evt, NULL, "fkqlevel", inty);

	suc = fai = 0;
	timer_start();
	int i;
	for (i = 0; i < times; i++) {
		ret = mevent_trigger(evt);
		if (PROCESS_OK(ret)) {
			if (times <= 100) {
				printf("process success %d\n", ret);
				data_cell_dump(evt->rcvdata);
			}
			suc++;
		} else {
			if (times <= 100) {
				printf("process failure %d!\n", ret);
			}
			fai++;
		}
	}
	s_elapsed = timer_stop();
	printf("%lu\n", s_elapsed);
	printf("suc %d fai %d\n", suc, fai);

	mevent_free(evt);
	return 0;
}
