
#include <stdio.h>        /* vsprintf() */
#include <stdarg.h>
#include <sys/types.h>         /* open() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>         /* write() */
#include <string.h>        /* strcmp(), strerror() */
#include <errno.h>        /* errno */
#include <time.h>        /* time() and friends */

#include "log.h"
#include "common.h"
#include "mheads.h"
#include "ClearSilver.h"
#include "config.h"

int config_parse_file(const char *file, HDF **cfg)
{
    NEOERR *err;

    err = hdf_init(cfg);
    RETURN_V_NOK(err, 0);
    
    err = hdf_read_file(*cfg, file);
    RETURN_V_NOK(err, 0);

    return 1;
}

void config_cleanup(HDF **config)
{
    if (*config == NULL) return;
    hdf_destroy(config);
}
