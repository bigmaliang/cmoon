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

	mevent_t *evt = mevent_init("unknown");
	if (evt == NULL) {
		printf("init error\n");
		return 1;
	}

	mevent_free(evt);
	return 0;
}
