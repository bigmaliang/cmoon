#ifndef __LDB_H__
#define __LDB_H__
#include "mheads.h"

__BEGIN_DECLS

#define TABLE_RLS_USER	(hdf_get_value(g_cfg, "Db.Table.release_user", "rls_user_4"))

/*
 * col, table, condition MUST be string literal, not variable
 * "tjt_%d" OK
 * char *table NOK
 * e.g.
 * LDB_QUERY_RAW(dbtjt, "tjt_%d", TJT_QUERY_COL, "fid=%d ORDER BY uptime "
 * " LIMIT %d OFFSET %d", NULL, aid, fid, count, offset);
 */
#define LDB_QUERY_RAW(conn, table, col, condition, sfmt, ...)           \
	mdb_exec(conn, NULL, "SELECT " col " FROM " table " WHERE " condition ";", \
			 sfmt, ##__VA_ARGS__)

void ldb_opfinish(int ret, HDF *hdf, mdb_conn *conn,
				  char *target, char *url, bool header);
void ldb_opfinish_json(int ret, HDF *hdf, mdb_conn *conn, time_t second);

int  ldb_init(HASH **dbh);
void ldb_destroy(HASH *dbh);

__END_DECLS
#endif	/* __LDB_H__ */
