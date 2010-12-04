#ifndef __MDB_H__
#define __MDB_H__
#include "mheads.h"

__BEGIN_DECLS

/*
 * ========================
 * ==== user attention ====
 * ========================
 * except mdb_init return RET_RBTOP_* for cgi use
 * others mdb API return MDB_ERR_* for success check.
 *
 * some function return !MDB_ERR_NONE, but don't so terriable for most situation.
 * e.g. NORESULT for _get
 */

/*
 * ==========================
 * ==== hacker attention ====
 * ==========================
 * if you return !MDB_ERR_NONE, please make sure set the errcode
 * and vice versa, if you set errcode, make sure return !MDB_ERR_NONE
 */

typedef struct _mdb_conn mdb_conn;
typedef struct _mdb_query mdb_query;

enum _mdb_errors
{
  MDB_ERR_NONE = 0,
  
  MDB_ERR_NIMPLEM,
  MDB_ERR_API_TURN,
  MDB_ERR_INPUTE,

  MDB_ERR_RESULTE,
  MDB_ERR_NORESULT,
  MDB_ERR_RESULT_ENDED,

  MDB_ERR_UNIQUE_VIOLATION,
  MDB_ERR_NOT_NULL_VIOLATION,
  MDB_ERR_OTHER,
  
  MDB_ERR_INIT = 100,
  MDB_ERR_MEMORY_ALLOC
};

int  mdb_init(mdb_conn **conn, char *dsn);
void mdb_destroy(mdb_conn *conn);
const char* mdb_get_backend(mdb_conn *conn);
const char* mdb_get_errmsg(mdb_conn *conn);
int  mdb_get_errcode(mdb_conn *conn);
void mdb_set_error(mdb_conn *conn, int code, const char* msg);
void mdb_clear_error(mdb_conn *conn);
int mdb_begin(mdb_conn *conn);
int mdb_commit(mdb_conn *conn);
int mdb_rollback(mdb_conn *conn);
int mdb_finish(mdb_conn *conn);

/*
 * in most situation, mdb_x() functions sutisfied
 */
int mdb_exec(mdb_conn *conn, int *affectrow, const char* sql_fmt, const char* fmt, ...);
int mdb_put(mdb_conn *conn, const char* fmt, ...);
int mdb_get(mdb_conn *conn, const char* fmt, ...);
/* store data to res[], fmt must be '[sS]+' */
int mdb_geta(mdb_conn *conn, const char* fmt, char* res[]);
int mdb_set_row(HDF *hdf, mdb_conn* conn, char *cols, char *prefix);
/*
 * set db rows result into hdf
 * hdf	 :OUT result store into
 * conn  :IN db
 * cols  :IN SET which colums(hdf key) {aid, aname}, NULL for single col
 * prefix:IN store in hdf whith prefix (Output)
 * keycol:IN use which colum as hdf's key(exec, not cols), start with 0. -1 form number
 */
int mdb_set_rows(HDF *hdf, mdb_conn* conn, char *cols,
				 char *prefix, int keycol);
int mdb_get_rows(mdb_conn *conn);
int mdb_get_affect_rows(mdb_conn *conn);
int mdb_get_last_id(mdb_conn *conn, const char* seq_name);

/*
 * mdb_x_apart() functions supplyed for multiply query 
 */
int mdb_exec_apart(mdb_conn *conn, mdb_query **pquery,
				   int *affectrow, const char* sql_fmt, const char* fmt, ...);
int mdb_put_apart(mdb_query *query, const char* fmt, ...);
int mdb_get_apart(mdb_query *query, const char* fmt, ...);
int mdb_geta_apart(mdb_query *query, const char* fmt, char* res[]);
int mdb_get_rows_apart(mdb_query *query);
int mdb_get_affect_rows_apart(mdb_query *query);
int mdb_get_last_id_apart(mdb_query *query, const char* seq_name);

void mdb_opfinish(int ret, HDF *hdf, mdb_conn *conn,
				  char *target, char *url, bool header);
void mdb_opfinish_json(int ret, HDF *hdf, mdb_conn *conn);


#define PRE_DBOP(hdf, conn)								\
	if (hdf == NULL) {									\
		mtc_err("hdf is null");							\
		return RET_RBTOP_HDFNINIT;						\
	}													\
	if (MDB_CONN_BAD(conn)) {							\
		mtc_err("conn bad %d", mdb_get_errcode(conn));	\
		return RET_RBTOP_DBNINIT;						\
	} else if (mdb_get_errcode(conn) != MDB_ERR_NONE) {	\
		mtc_err("conn err %s", mdb_get_errmsg(conn));	\
		mdb_clear_error(conn);							\
	}

#define PRE_DBOP_NRET(hdf, conn)						\
	if (hdf == NULL) {									\
		mtc_err("hdf is null");							\
		return;											\
	}													\
	if (MDB_CONN_BAD(conn)) {							\
		mtc_err("conn bad %d", mdb_get_errcode(conn));	\
		return;											\
	} else if (mdb_get_errcode(conn) != MDB_ERR_NONE) {	\
		mtc_err("conn err %s", mdb_get_errmsg(conn));	\
		mdb_clear_error(conn);							\
	}

#define MDB_EXEC_RBT(conn, affrow, sqlfmt, fmt, ...)				\
	do {															\
		if (mdb_exec(conn, affrow, sqlfmt, fmt, ##__VA_ARGS__)) {	\
			mtc_err("exec failure %s", mdb_get_errmsg(conn));		\
			return RET_RBTOP_DBE;									\
		}															\
	} while (0)
#define MDB_EXEC_EVT(conn, affrow, sqlfmt, fmt, ...)				\
	do {															\
		if (mdb_exec(conn, affrow, sqlfmt, fmt, ##__VA_ARGS__)) {	\
			mtc_err("exec failure %s", mdb_get_errmsg(conn));		\
			return REP_ERR_DB;										\
		}															\
	} while (0)

/*
 * col, table, condition MUST be string literal, not variable
 * "tjt_%d" OK
 * char *table NOK
 * e.g.
 * LDB_QUERY_RAW(dbtjt, "tjt_%d", TJT_QUERY_COL, "fid=%d ORDER BY uptime "
 * " LIMIT %d OFFSET %d", NULL, aid, fid, count, offset);
 */
#define MDB_QUERY_RAW(conn, table, col, condition, sfmt, ...)           \
	mdb_exec(conn, NULL, "SELECT " col " FROM " table " WHERE " condition ";", \
			 sfmt, ##__VA_ARGS__)

#define MDB_CONN_BAD(conn) (mdb_get_errcode(conn) > MDB_ERR_OTHER)

__END_DECLS
#endif	/* __MDB_H__ */
