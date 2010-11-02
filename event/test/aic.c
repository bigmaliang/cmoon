#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mevent.h"
#include "timer.h"

int main(int argc, char *argv[])
{
	int ret;

	mevent_t *evt = mevent_init_plugin("dyn");
	if (evt == NULL) {
		printf("init error\n");
		return 1;
	}

	hdf_set_value(evt->hdfsnd, "aname", "kol");
	hdf_set_value(evt->hdfsnd, "uname", "ru_PGFOV");
	hdf_set_value(evt->hdfsnd, "to", "kol");

	ret = mevent_trigger(evt, NULL, 1001, FLAGS_SYNC);
	if (PROCESS_OK(ret))
		hdf_dump(evt->hdfrcv, NULL);
	else
		printf("process failure %d\n", ret);
	
	mevent_free(evt);
	return 0;
}
