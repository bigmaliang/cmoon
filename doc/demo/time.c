#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>

#include "mheads.h"

static struct timeval tv_s, tv_e;

static void timer_start(void) {
    gettimeofday(&tv_s, NULL);
}

static unsigned long timer_stop(void) {
    gettimeofday(&tv_e, NULL);
    return (tv_e.tv_sec - tv_s.tv_sec) * 1000000ul
        + (tv_e.tv_usec - tv_s.tv_usec);
}


HDF *g_cfg = NULL;

int main()
{
    long int x = 0, tm;
    int i;
    
    timer_start();
    for (i = 0; i < 1; i++) {
        x = time(NULL);
    }
    tm = timer_stop();

    printf("use %lu usec %d\n", tm, ((38000 + 37999) / 38000) % 229);
    
    return 0;
}
