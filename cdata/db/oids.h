#ifndef __OIDS_H__
#define __OIDS_H__

#include "mheads.h"

__BEGIN_DECLS

#define IDS_MAX_DBS		16

typedef struct {
	int num;
	fdb_t **dbs;
} ids_db_t;

int ids_dbt_init(ids_db_t **dbt);
void dbt_free(ids_db_t *dbt);
int ids_get_data(HDF *hdf, ids_db_t *dbt);

__END_DECLS
#endif	/* __OIDS_H__ */
