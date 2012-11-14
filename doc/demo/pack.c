#include "mheads.h"
#include "packet.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv, char **envp)
{
    unsigned char buf[2048];
    int blen = 2048, len;
    char *s;
    HDF *hdf;
    
    //mconfig_parse_file("/tpl/oms.hdf", &g_cfg);

    mtimer_start();
    for (int i = 0; i < 100000; i++) {
        memset(buf, 2048, 0x0);
        len = pack_hdf(g_cfg, buf, blen);
        unpack_hdf(buf, len, &hdf);
        s = hdf_get_value(hdf, "manual.Layout", NULL);
        hdf_destroy(&hdf);
    }
    mtimer_stop(NULL);

    return 0;
}
