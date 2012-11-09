#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mevent.h"

HDF *g_cfg = NULL;
HASH *g_datah = NULL;

int main(int argc, char *argv[])
{
    mevent_t *evt;
    int ret;
    char plugin[64] = "uic";
    int cmd;
    char key[64], val[64];
    
    cmd = 100;
    
    if (argc > 1) {
        strncpy(plugin, argv[1], sizeof(plugin));
    } else {
        printf("Usage: %s [PLUGIN] [COMMAND] [key] [val]\n", argv[0]);
        return 1;
    }
    if (argc > 2) {
        cmd = atoi(argv[2]);
    }
    if (argc > 3) {
        strncpy(key, argv[3], sizeof(key));
    }
    if (argc > 4) {
        strncpy(val, argv[4], sizeof(val));
    }

    evt = mevent_init_plugin(plugin);
    hdf_set_value(evt->hdfsnd, "cachekey", key);
    hdf_set_value(evt->hdfsnd, "cacheval", val);

    ret = mevent_trigger(evt, NULL, cmd, FLAGS_SYNC);
    if (PROCESS_OK(ret)) {
        printf("process success %d\n", ret);
        hdf_dump(evt->hdfrcv, NULL);
    } else {
        printf("process failure %d!\n", ret);
    }

    mevent_free(evt);
    return 0;
}
