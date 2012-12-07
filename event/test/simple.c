#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mevent.h"

HDF *g_cfg = NULL;
HASH *g_datah = NULL;

int main(int argc, char *argv[])
{
    unsigned long s_elapsed;
    int times, suc, fai, busy;
    char host[64], sql[1024];
    int ret;

    if (argc > 1) {
        times = atoi(argv[1]);
    } else {
        printf("Usage: %s TIMES [HOST] [SQL]\n", argv[0]);
        return 1;
    }
    if (argc > 2) {
        strncpy(host, argv[2], sizeof(host));
    } else {
        strcpy(host, "127.0.0.1");
    }
    if (argc > 3) {
        strncpy(sql, argv[3], sizeof(sql));
    }

    mevent_t *evt = mevent_init("db_community");
    if (evt == NULL) {
        printf("init error\n");
        return 1;
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 800000;
    
    mevent_add_tcp_server(evt, host, 26011, NULL, &tv);
    
    int i;
    suc = fai = busy = 0;
    for (i = 0; i < times; i++) {
        hdf_set_value(evt->hdfsnd, "sqls", sql);
        ret = mevent_trigger(evt, NULL, REQ_CMD_NONE, FLAGS_NONE);
        if (ret != 0 && ret < REP_ERR) {
            hdf_dump(evt->hdfrcv, NULL);
            suc++;
        } else if (ret == REP_ERR_BUSY) {
            printf("process busy!\n");
            busy++;
        } else
            fai++;
    }
    printf("suc %d fai %d busy %d\n", suc, fai, busy);
    
    mevent_free(evt);
    return 0;
}
