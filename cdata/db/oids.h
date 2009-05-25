#ifndef __OIDS_H__
#define __OIDS_H__

#include "fheads.h"

#define IDS_DOMAIN_NUM		1

int ids_fdb_init(fdb_t **fdb);
int ids_get_data(HDF *hdf, fdb_t *fdb);

#endif	/* __OIDS_H__ */
