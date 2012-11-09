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

#ifndef __MTIMER_H__
#define __MTIMER_H__

#include "mheads.h"

__BEGIN_DECLS

#define TIMER_NUM 64
#define TIMER_MSG_LEN 256

void mtimer_start(void);
unsigned long mtimer_stop(char *fmt, ...)
    ATTRIBUTE_PRINTF(1, 2);

__END_DECLS
#endif    /* __MTIMER_H__ */
