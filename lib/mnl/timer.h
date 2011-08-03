
/*
 * A simple timer for measuring delays.
 * Alberto Bertogli (albertito@blitiri.com.ar) - September/2006
 *
 * Use it like this:
 *     unsigned long elapsed;
 *     ...
 *     timer_start();
 *     ... [code] ...
 *     elapsed = timer_stop();
 *     ...
 *     printf("Time elapsed: %lu", elapsed);
 *
 * Nested timers are not supported. The result is in usecs.
 *
 * 1000000 usecs == 1 sec
 */


#ifndef _TIMER_H
#define _TIMER_H

#include <sys/time.h>

static struct timeval tv_s, tv_e;

static void timer_start(void) {
    gettimeofday(&tv_s, NULL);
}

static unsigned long timer_stop(void) {
    gettimeofday(&tv_e, NULL);
    return (tv_e.tv_sec - tv_s.tv_sec) * 1000000ul
        + (tv_e.tv_usec - tv_s.tv_usec);
}

#endif

