#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mheads.h"
#include "mevent.h"

HDF *g_cfg = NULL;
HASH *g_datah = NULL;

int main(int argc, char *argv[])
{
    unsigned long elapsed;
    int ret;

    mevent_t *evt = mevent_init_plugin("skeleton");
    if (evt == NULL) {
        printf("init error\n");
        return 1;
    }

    mtimer_start();
    
    for (int i = 0; i < 100000; i++) {
        ret = mevent_trigger(evt, NULL, REQ_CMD_STATS, FLAGS_NONE);
        if (PROCESS_OK(ret))
            //hdf_dump(evt->hdfrcv, NULL);
            ;
        else
            printf("process failure %d\n", ret);
    }
    
    mtimer_stop(NULL);

    printf("Time elapsed: %lu usecs\n", elapsed);
    
    mevent_free(evt);
    return 0;
}
