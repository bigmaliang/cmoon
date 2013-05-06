#include "apev.h"
#include "net.h"

#include "main.h"

HDF *g_cfg = NULL;

extern struct event_entry ext_entry;

int main(int argc, char *argv[])
{
    NEOERR *err;

    static struct setting {
        char *conffname;
        int foreground;
    } myset = {
        .conffname = "./config.hdf",
        .foreground = 0,
    };

    int c;
    while ((c = getopt(argc, argv, "c:f")) != -1) {
        switch(c) {
        case 'c':
            myset.conffname = strdup(optarg);
            break;
        case 'f':
            myset.foreground = 1;
            break;
        }
    }

    err = mconfig_parse_file(myset.conffname, &g_cfg);
    DIE_NOK_MTL(err);
    
    mtc_init(hdf_get_value(g_cfg, "V.logfile", "/tmp/mevent"));

    err = nerr_init();
    RETURN_V_NOK(err, 1);

    err = parse_regist_v(&ext_entry);
    RETURN_V_NOK(err, 1);

    if (!myset.foreground) {
        pid_t pid = fork();
        if (pid > 0) return 0;
        else if (pid < 0) {
            perror("Error in fork");
            return 1;
        }
        close(0);
        setsid();
    }

    mtc_foo("start v done");

    net_go();

    mconfig_cleanup(&g_cfg);

    return 0;
}
