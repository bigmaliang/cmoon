#ifndef __LDB_H__
#define __LDB_H__
#include "mheads.h"

__BEGIN_DECLS

#define DB_DSN			(mcfg_getvalue("dbdsn", "pgsql:dbname=mn_user host=localhost user=mner password=loveu"))
#define DB_DSN_SYS		(mcfg_getvalue("dbdsn_sys", "pgsql:dbname=mn_sys host=localhost user=mner password=loveu"))
#define TABLE_RLS_USER	(mcfg_getvalue("table", "rls_user_4"))

void ldb_opfinish(int ret, HDF *hdf, mdb_conn *conn,
				  char *target, char *url, bool header);
void ldb_opfinish_json(int ret, HDF *hdf, mdb_conn *conn);

__END_DECLS
#endif	/* __LDB_H__ */
