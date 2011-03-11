#include "net.h"
#include "udp.h"

static void time_up(int fd, short flags, void* arg)
{
	struct event *ev = (struct event*)arg;
	struct event_base *base = ev->ev_base;
	struct timeval t = {.tv_sec = 100, .tv_usec = 0};

	event_del(ev);
	event_set(ev, -1, 0, time_up, ev);
	event_base_set(base, ev);
	event_add(ev, &t);
	
	//event_set(ev, -1, 0, time_up, ev);

	fprintf(stderr, "current time %u\n", time(NULL));
}

void net_go()
{
	struct event_base *base;
	struct event ev, ev_clock;
	struct timeval t = {.tv_sec = 100, .tv_usec = 0};
	int fd = -1;

	fd = udp_init();

	base = event_base_new();

	event_set(&ev, fd, EV_READ | EV_PERSIST, udp_recv, &ev);
	event_base_set(base, &ev);
	event_add(&ev, NULL);

	event_set(&ev_clock, -1, 0, time_up, &ev_clock);
	event_base_set(base, &ev_clock);
	event_add(&ev_clock, &t);

	event_base_loop(base, 0);

	event_del(&ev);
	event_del(&ev_clock);
	
	udp_close(fd);
}
