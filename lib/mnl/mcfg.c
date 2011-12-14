#include "mheads.h"

NEOERR* mcfg_parse_file(const char *file, HDF **cfg)
{
    NEOERR *err;

    err = hdf_init(cfg);
    if (err != STATUS_OK) return nerr_pass(err);
    
    err = hdf_read_file(*cfg, file);
    if (err != STATUS_OK) return nerr_pass(err);

    return STATUS_OK;
}

void mcfg_cleanup(HDF **config)
{
    if (*config == NULL) return;
    hdf_destroy(config);
}
