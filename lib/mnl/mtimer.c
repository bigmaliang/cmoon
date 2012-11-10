#include "mheads.h"

unsigned long elapsed = 0;

static struct timeval tv_s[TIMER_NUM], tv_e[TIMER_NUM];
static int current_timer = 0;
static char timer_msg[TIMER_MSG_LEN] = {0};

void mtimer_start(void)
{
    if (current_timer < TIMER_NUM) {
        gettimeofday(&tv_s[current_timer], NULL);
        current_timer++;
    }
}

unsigned long mtimer_stop(char *fmt, ...)
{
    if (current_timer > 0) {
        current_timer--;
        gettimeofday(&tv_e[current_timer], NULL);
        elapsed = (tv_e[current_timer].tv_sec - tv_s[current_timer].tv_sec) * 1000000ul +
            (tv_e[current_timer].tv_usec - tv_s[current_timer].tv_usec);
    }

    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(timer_msg, TIMER_MSG_LEN, fmt, ap);
        va_end(ap);
        
        mtc_foo("%s : %lu usecs", timer_msg, elapsed);
    }
    
    return elapsed;
}
