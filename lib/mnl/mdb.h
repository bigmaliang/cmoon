#ifndef __MDB_H__
#define __MDB_H__
#include "mheads.h"

__BEGIN_DECLS

typedef struct _mdb_conn mdb_conn;
typedef struct _mdb_driver mdb_driver;

#define MDB_DV_NUM	3
extern mdb_driver mysql_driver;
extern mdb_driver sqlite_driver;
extern mdb_driver pgsql_driver;

struct _mdb_conn
{
	char* dsn;
	char* sql;
	mdb_driver* driver;
	bool in_transaction;
};

struct _mdb_driver
{
	char* name;
	
	NEOERR* (*connect)(const char* dsn, mdb_conn **rconn);
	void (*disconnect)(mdb_conn* conn);
	
	NEOERR* (*begin)(mdb_conn* conn);
	NEOERR* (*commit)(mdb_conn* conn);
	NEOERR* (*rollback)(mdb_conn* conn);
	
	NEOERR* (*query_fill)(mdb_conn* conn, const char* sql_string);
	NEOERR* (*query_getv)(mdb_conn* conn, const char* fmt, va_list ap);
	NEOERR* (*query_putv)(mdb_conn* conn, const char* fmt, va_list ap);
	NEOERR* (*query_geta)(mdb_conn* conn, const char* fmt, char *res[]);
	
	int (*query_get_rows)(mdb_conn* conn);
	int (*query_get_affect_rows)(mdb_conn* conn);
	int (*query_get_last_id)(mdb_conn* conn, const char* seq_name);
};

NEOERR* mdb_init(mdb_conn **conn, char *dsn);
void mdb_destroy(mdb_conn *conn);
const char* mdb_get_backend(mdb_conn *conn);
const char* mdb_get_errmsg(mdb_conn *conn);
NEOERR* mdb_begin(mdb_conn *conn);
NEOERR* mdb_commit(mdb_conn *conn);
NEOERR* mdb_rollback(mdb_conn *conn);
NEOERR* mdb_finish(mdb_conn *conn);

NEOERR* mdb_exec(mdb_conn *conn, int *affectrow, const char* sql_fmt,
				 const char* fmt, ...);
NEOERR* mdb_put(mdb_conn *conn, const char* fmt, ...);
NEOERR* mdb_get(mdb_conn *conn, const char* fmt, ...);
/* store data to res[], fmt must be '[sS]+' */
NEOERR* mdb_geta(mdb_conn *conn, const char* fmt, char* res[]);
NEOERR* mdb_set_row(HDF *hdf, mdb_conn* conn, char *cols, char *prefix);
/*
 * set db rows result into hdf
 * hdf	 :OUT result store into
 * conn  :IN db
 * cols  :IN SET which colums(hdf key) {aid, aname}, NULL for single col
 * prefix:IN store in hdf whith prefix (Output)
 * keycol:IN use which colum as hdf's key(exec, not cols), start with 0. -1 for number
 */
NEOERR* mdb_set_rows(HDF *hdf, mdb_conn* conn, char *cols,
					 char *prefix, int keycol);
int mdb_get_rows(mdb_conn *conn);
int mdb_get_affect_rows(mdb_conn *conn);
int mdb_get_last_id(mdb_conn *conn, const char* seq_name);


/*
 * col, table, condition MUST be string literal, not variable
 * "tjt_%d" OK
 * char *table NOK
 * e.g.
 * LDB_QUERY_RAW(dbtjt, "tjt_%d", TJT_QUERY_COL, "fid=%d ORDER BY uptime "
 * " LIMIT %d OFFSET %d", NULL, aid, fid, count, offset);
 */
#define MDB_QUERY_RAW(conn, table, col, condition, sfmt, ...)			\
	do {																\
		err = mdb_exec(conn, NULL, "SELECT " col " FROM " table " WHERE " condition ";", \
					   sfmt, ##__VA_ARGS__);							\
		if (err != STATUS_OK) return nerr_pass(err);					\
	} while (0)
#define MDB_EXEC(conn, affrow, sqlfmt, fmt, ...)					\
	do {															\
		err = mdb_exec(conn, affrow, sqlfmt, fmt, ##__VA_ARGS__);	\
		if (err != STATUS_OK) return nerr_pass(err);				\
	} while (0)
#define MDB_EXEC_TS(conn, affrow, sqlfmt, fmt, ...)					\
	do {															\
		err = mdb_exec(conn, affrow, sqlfmt, fmt, ##__VA_ARGS__);	\
		if (err != STATUS_OK) {										\
			mdb_rollback(conn);										\
			return nerr_pass(err);									\
		} \
	} while (0)

__END_DECLS
#endif	/* __MDB_H__ */
