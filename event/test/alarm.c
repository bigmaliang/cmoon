#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mevent.h"

HDF *g_cfg = NULL;
HASH *g_datah = NULL;

int main(int argc, char *argv[])
{
    mevent_t *evt = mevent_init("unknown");
    if (evt == NULL) {
        printf("init error\n");
        return 1;
    }

    mevent_free(evt);
    return 0;
}
