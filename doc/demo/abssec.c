#include "mheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv)
{
    time_t now;

    now = mutil_get_abssec("%Y-%m-%d", argv[1]);

    printf("%ld\n", now);
    
    return 0;
}
