#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mevent.h"
#include "timer.h"

int main(int argc, char *argv[])
{
	int ret;

	mevent_t *evt = mevent_init("Reserve.Status");
	if (evt == NULL) {
		printf("init error\n");
		return 1;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 800000;
	
	mevent_add_udp_server(evt, "127.0.0.1", 26000, NULL, &tv);
	ret = mevent_trigger(evt, NULL, REQ_CMD_STATS, FLAGS_NONE);
	if (ret != 0 && ret < REP_ERR) {
		hdf_dump(evt->hdfrcv, NULL);
	} else if (ret == REP_ERR_BUSY) {
		printf("process busy!\n");
	} else
		printf("process failure %d\n", ret);
	
	mevent_free(evt);
	return 0;
}
