#include "apev.h"
#include "net.h"

#include "main.h"

static void time_up(int fd, short flags, void* arg)
{
	struct event *ev = (struct event*)arg;
	struct event_base *base = ev->ev_base;
	struct timeval t = {.tv_sec = 10, .tv_usec = 0};
	static bool initialized = false;

	if (initialized) event_del(ev);
	else initialized = true;
	
	event_set(ev, -1, 0, time_up, ev);
	event_base_set(base, ev);
	event_add(ev, &t);

	mtc_dbg("current time %ld", time(NULL)); 
}

static void net_read(int fd, short event, void *arg)
{
	NEOERR *err = udps_recv(fd, 0, NULL);
	TRACE_NOK(err);
}

void net_go()
{
	struct event_base *base;
	struct event ev, ev_clock;
	struct timeval t = {.tv_sec = 10, .tv_usec = 0};
	int fd = -1;

	fd = udps_init(hdf_get_value(g_cfg, "V.ip", "127.0.0.1"),
				   hdf_get_int_value(g_cfg, "V.port", 50000));
	
	base = event_base_new();

	event_set(&ev, fd, EV_READ | EV_PERSIST, net_read, &ev);
	event_base_set(base, &ev);
	event_add(&ev, NULL);

	event_set(&ev_clock, -1, 0, time_up, &ev_clock);
	event_base_set(base, &ev_clock);
	event_add(&ev_clock, &t);

	event_base_loop(base, 0);

	event_del(&ev);
	event_del(&ev_clock);
	
	udps_close(fd);
}
