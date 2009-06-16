#ifndef __LDB_H__
#define __LDB_H__
#include "mheads.h"

__BEGIN_DECLS

#define TABLE_RLS_USER	(hdf_get_value(g_cfg, "Db.Table.release_user", "rls_user_4"))

void ldb_opfinish(int ret, HDF *hdf, mdb_conn *conn,
				  char *target, char *url, bool header);
void ldb_opfinish_json(int ret, HDF *hdf, mdb_conn *conn);

int  ldb_init(HASH **dbh);
void ldb_destroy(HASH *dbh);

__END_DECLS
#endif	/* __LDB_H__ */
