#ifndef __MDB_PRIV_H__
#define __MDB_PRIV_H__

#include "mheads.h"

__BEGIN_DECLS

#define CONN_DRIVER(c)	c->driver
#define QUERY_DRIVER(q)	q->conn->driver
#define QUERY_CONN(q)	q->base->conn

#define CONN_RETURN_IF_INVALID(c)					\
	if (mdb_get_errcode(c) != MDB_ERR_NONE) {		\
		mtc_err("conn err %s", mdb_get_errmsg(c));	\
		return;										\
	}
#define CONN_RETURN_VAL_IF_INVALID(c, val)			\
	if (mdb_get_errcode(c) != MDB_ERR_NONE) { 		\
		mtc_err("conn err %s", mdb_get_errmsg(c));	\
		return val;									\
	}
#define QUERY_RETURN_IF_INVALID(q)								\
	if (q == NULL || mdb_get_errcode(q->conn) != MDB_ERR_NONE)	\
		return;
#define QUERY_RETURN_VAL_IF_INVALID(q, val)						\
	if (q == NULL || mdb_get_errcode(q->conn) != MDB_ERR_NONE)	\
		return val;

typedef struct _mdb_driver mdb_driver;

struct _mdb_conn
{
  char* dsn;
  int errcode;
  char* errmsg;
  mdb_driver* driver;
/*   mdb_query* query; */
  ULIST *queries;
  bool in_transaction;
};

struct _mdb_query
{
  mdb_conn* conn;
  char* sql;
};

struct _mdb_driver
{
  char* name;

  mdb_conn* (*connect)(const char* dsn);
  void (*disconnect)(mdb_conn* conn);

  int (*begin)(mdb_conn* conn);
  int (*commit)(mdb_conn* conn);
  int (*rollback)(mdb_conn* conn);

  mdb_query* (*query_new)(mdb_conn* conn, const char* sql_string);
  int (*query_fill)(mdb_query* query, const char* sql_string);
  void (*query_free)(mdb_query* query);

  int (*query_getv)(mdb_query* query, const char* fmt, va_list ap);
  int (*query_putv)(mdb_query* query, const char* fmt, va_list ap);

  int (*query_get_rows)(mdb_query* query);
  int (*query_get_affect_rows)(mdb_query* query);
  int (*query_get_last_id)(mdb_query* query, const char* seq_name);
};

extern mdb_driver sqlite_driver;
#ifndef DROP_PG
extern mdb_driver pgsql_driver;
#define MDB_DV_NUM	2
#else
#define MDB_DV_NUM	1
#endif

mdb_conn* mdb_connect(const char* dsn);
void mdb_disconnect(mdb_conn* conn);

mdb_query* mdb_query_new(mdb_conn* conn, const char* sql_fmt, ...);
int mdb_query_fill(mdb_query* query, const char* sql_fmt, ...);
void mdb_query_free(void *data);
int mdb_query_putv(mdb_query* query, const char* fmt, va_list ap);
int mdb_query_getv(mdb_query* query, const char* fmt, va_list ap);

int mdb_query_put(mdb_query* query, const char* fmt, ...);
int mdb_query_get(mdb_query* query, const char* fmt, ...);
int mdb_query_get_rows(mdb_query* query);
int mdb_query_get_affect_rows(mdb_query* query);
int mdb_query_get_last_id(mdb_query* query, const char* seq_name);

__END_DECLS
#endif	/* __MDB_PRIV_H__ */
