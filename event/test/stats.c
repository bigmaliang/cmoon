#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mevent.h"
#include "timer.h"

int main(int argc, char *argv[])
{
	uint32_t errcode;
	int ret;

	mevent_t *evt = mevent_init();
	if (evt == NULL) {
		printf("init error\n");
		return 1;
	}

	mevent_add_udp_server(evt, "127.0.0.1", 26010);
	mevent_chose_plugin(evt, "Reserve.Status", REQ_CMD_STATS, FLAGS_NONE);
	ret = mevent_trigger(evt, &errcode);
	if (ret == REP_OK) {
		data_cell_dump(evt->rcvdata);
	} else if (ret == REP_ERR) {
		if (errcode == ERR_BUSY) {
			printf("process busy %d!\n", errcode);
		} else {
			printf("process error %d!\n", errcode);
		}
	} else
		printf("process failure %d\n", ret);
	
	mevent_free(evt);
	return 0;
}
