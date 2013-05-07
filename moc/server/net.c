#include "apev.h"
#include "net.h"

#include "main.h"

volatile time_t g_time;

static void ext_check_zomble()
{
    char *key  = NULL;
    
    SnakeEntry *s = (SnakeEntry*)hash_next(stbl, (void**)&key);

    while (s) {
        if (g_time - s->idle > ZOMBLE_SEC) {
            if (s->state == RUNNING) {
                s->state = DIED;
                 mtc_err("%s died", s->name);
                //alarm();
                //notice_other();
            }
        } else {
            if (s->state == DIED) {
                s->state = RUNNING;
                 mtc_foo("%s alived", s->name);
                //notice_other();
            }
        }

        s = hash_next(stbl, (void**)&key);
    }

    ext_snake_sort();
}

static void time_up(int fd, short flags, void* arg)
{
    struct event *ev = (struct event*)arg;
    struct event_base *base = ev->ev_base;
    struct timeval t = {.tv_sec = TIMEUP_SEC, .tv_usec = 0};
    static bool initialized = false;
    static int intime = 0;

    if (initialized) event_del(ev);
    else initialized = true;
    
    event_set(ev, -1, 0, time_up, ev);
    event_base_set(base, ev);
    event_add(ev, &t);

    g_time = time(NULL);

    if (++intime > (ZOMBLE_SEC / TIMEUP_SEC) ) {
        intime = 0;
        ext_check_zomble();
    }
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
    struct timeval t = {.tv_sec = TIMEUP_SEC, .tv_usec = 0};
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
