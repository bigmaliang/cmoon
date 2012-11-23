#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <event.h>
#include "mevent.h"

static struct event_base *me;
static struct event e;

HDF *g_cfg = NULL;

void f(const int fd, const short which, void *arg)
{
    struct timeval t = {.tv_sec = 1, .tv_usec = 0};
    static bool init = false;

    if (init) {
        evtimer_del(&e);
    } else {
        init = true;
    }
    
    evtimer_set(&e, f, NULL);
    event_base_set(me, &e);
    evtimer_add(&e, &t);
    
    printf("xxx\n");
}

int main()
{
    me = event_init();

    f(0, 0, NULL);

    event_base_loop(me, 0);
    
    return 0;
}

