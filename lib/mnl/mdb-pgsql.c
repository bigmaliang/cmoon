#include "mdb-priv.h"

#ifndef DROP_PG
#include <libpq-fe.h>

struct _mdb_conn_pgsql
{
	mdb_conn base;
	PGconn* pg;
};

struct _mdb_query_pgsql
{
	mdb_query base;
	PGresult* pg_res;
	int row_no;
};

#define CONN(c) ((struct _mdb_conn_pgsql*)(c))
#define QUERY(c) ((struct _mdb_query_pgsql*)(c))

void notices_black_hole(void* arg, const char* message)
{
	;
}

static mdb_conn* pgsql_mdb_connect(const char* dsn)
{
	struct _mdb_conn_pgsql* conn;
  
	conn = calloc(1, sizeof(struct _mdb_conn_pgsql));
	if (conn == NULL) {
		mtc_die("calloc for ms connect failure.");
		return NULL;
	}
	conn->pg = PQconnectdb(dsn);
	if (PQstatus(conn->pg) == CONNECTION_BAD)
		mdb_set_error((mdb_conn*)conn, MDB_ERR_OTHER, PQerrorMessage(conn->pg));
	else
		PQsetNoticeProcessor(conn->pg, notices_black_hole, NULL);

	return (mdb_conn*)conn;
}

static void pgsql_mdb_disconnect(mdb_conn* conn)
{
	if (CONN(conn)->pg)
		PQfinish(CONN(conn)->pg);
}

static int pgsql_mdb_begin(mdb_conn* conn)
{
	PGresult* res;
  
	res = PQexec(CONN(conn)->pg, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		mdb_set_error(conn, MDB_ERR_OTHER, PQresultErrorMessage(res));
		PQclear(res);
		return -1;
	}

	PQclear(res);
	return 0;
}

static int pgsql_mdb_commit(mdb_conn* conn)
{
	PGresult* res;
  
	res = PQexec(CONN(conn)->pg, "COMMIT");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		mdb_set_error(conn, MDB_ERR_OTHER, PQresultErrorMessage(res));
		PQclear(res);
		return -1;
	}

	PQclear(res);
	return 0;
}

static int pgsql_mdb_rollback(mdb_conn* conn)
{
	PGresult* res;
  
	res = PQexec(CONN(conn)->pg, "ROLLBACK");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		mdb_set_error(conn, MDB_ERR_OTHER, PQresultErrorMessage(res));
		PQclear(res);
		return -1;
	}

	PQclear(res);
	return 0;
}

static mdb_query* pgsql_mdb_query_new(mdb_conn* conn, const char* sql_string)
{
	struct _mdb_query_pgsql* query;
  
	query = calloc(1, sizeof(struct _mdb_query_pgsql));
	if (query == NULL) {
		mdb_set_error(conn, MDB_ERR_MEMORY_ALLOC, "calloc for query new failure.");
		return NULL;
	}
	query->base.conn = conn;
	if (sql_string)
		query->base.sql = strdup(sql_string);

	return (mdb_query*)query;
}

static int pgsql_mdb_query_fill(mdb_query* query, const char* sql_string)
{
	if (QUERY(query)->pg_res != NULL)
		PQclear(QUERY(query)->pg_res);
	QUERY(query)->pg_res = NULL;
	QUERY(query)->row_no = 0;
	
	if (query->sql)
		free(query->sql);
	query->sql = NULL;
	if (sql_string)
		query->sql = strdup(sql_string);

	return 0;
}

static void pgsql_mdb_query_free(mdb_query* query)
{
	/*
	 * if (p)
	 * if (p != NULL)
	 * both can judge a pointer valid, which one better?
	 */
	if (QUERY(query)->pg_res != NULL)
		PQclear(QUERY(query)->pg_res);
	QUERY(query)->pg_res = NULL;
	if (query->sql)
		free(query->sql);
	query->sql = NULL;
	free(query);
}

static int pgsql_mdb_query_getv(mdb_query* query, const char* fmt, va_list ap)
{
	PGresult* res = QUERY(query)->pg_res;
	int row_no = QUERY(query)->row_no;

	if (res == NULL)
		return -1;

	if (mdb_query_get_rows(query) <= 0) {
		mdb_set_error(query->conn, MDB_ERR_NONE, "attempt fetch emtpy result");
		return MDB_ERR_NORESULT;
	}
	if (row_no >= mdb_query_get_rows(query)) {
		mdb_set_error(query->conn, MDB_ERR_NONE, "last row has fetched");
		return MDB_ERR_RESULT_ENDED;
	}

	int param_count = fmt != NULL ? strlen(fmt) : 0;
	int i, col = 0;

	for (i = 0; i < param_count; i++) {
		if (fmt[i] == 's') {
			char** str_ptr = (char**)va_arg(ap, char**);
			if (PQgetisnull(res, row_no, col))
				*str_ptr = NULL;
			else
				*str_ptr = PQgetvalue(res, row_no, col);
			col++;
		} else if (fmt[i] == 'S')	{
			char** str_ptr = (char**)va_arg(ap, char**);
			if (PQgetisnull(res, row_no, col))
				*str_ptr = NULL;
			else
				*str_ptr = strdup(PQgetvalue(res, row_no, col));
			col++;
		} else if (fmt[i] == 'i')	{
			int* int_ptr = (int*)va_arg(ap, int*);
			if (!PQgetisnull(res, row_no, col))
				*int_ptr = atoi(PQgetvalue(res, row_no, col));
			col++;
		} else if (fmt[i] == '?')	{ // null flag
			int* int_ptr = (int*)va_arg(ap, int*);
			*int_ptr = PQgetisnull(res, row_no, col);
		} else {
			mdb_set_error(query->conn, MDB_ERR_OTHER, "Invalid format string.");
			va_end(ap);
			return -1;
		}
	}

	QUERY(query)->row_no++;
	return 0;
}

static int pgsql_mdb_query_geta(mdb_query* query, const char* fmt, char* r[])
{
	PGresult* res = QUERY(query)->pg_res;
	int row_no = QUERY(query)->row_no;

	if (res == NULL)
		return -1;

	if (mdb_query_get_rows(query) <= 0) {
		mdb_set_error(query->conn, MDB_ERR_NONE, "attempt fetch emtpy result");
		return MDB_ERR_NORESULT;
	}
	if (row_no >= mdb_query_get_rows(query)) {
		mdb_set_error(query->conn, MDB_ERR_NONE, "last row has fetched");
		return MDB_ERR_RESULT_ENDED;
	}

	int param_count = fmt != NULL ? strlen(fmt) : 0;
	int i, col = 0;

	for (i = 0; i < param_count; i++) {
		if (fmt[i] == 's') {
			if (PQgetisnull(res, row_no, col))
				r[i] = NULL;
			else
				r[i] = PQgetvalue(res, row_no, col);
		} else if (fmt[i] == 'S')	{
			if (PQgetisnull(res, row_no, col))
				r[i] = NULL;
			else
				r[i] = strdup(PQgetvalue(res, row_no, col));
		}
		col++;
	}

	QUERY(query)->row_no++;
	return 0;
}

static int pgsql_convert_error(const char *sqlstate)
{
	switch (strtoull(sqlstate, NULL, 0)) {
	case 23505:
		return MDB_ERR_UNIQUE_VIOLATION;
	case 23502:
		return MDB_ERR_NOT_NULL_VIOLATION;
	default:
		return MDB_ERR_OTHER;
	}
}

static int pgsql_mdb_query_putv(mdb_query* query, const char* fmt, va_list ap)
{
	int param_count = (fmt != NULL) ? strlen(fmt) : 0;
	char** param_values = calloc(param_count, sizeof(char*));
	int* free_list = calloc(param_count, sizeof(int));
	if (param_values == NULL || free_list == NULL) {
		mdb_set_error(query->conn, MDB_ERR_MEMORY_ALLOC, "calloc for query putv failure.");
		return -1;
	}
	int i, col = 0, retval = 0;
	PGresult* res;

	mtc_dbg("%s %s %d", query->sql, fmt, param_count);
	for (i = 0; i < param_count; i++) {
		int is_null = 0;

		if (fmt[i] == '?') {
			is_null = (int)va_arg(ap, int);
			i++;
		}

		if (fmt[i] == 's') {
			char* value = (char*)va_arg(ap, char*);
			if (!is_null)
				param_values[col] = value;
			col++;
		}
		else if (fmt[i] == 'i')	{
			int value = (int)va_arg(ap, int);
			char cvalue[64];
			if (!is_null) {
				sprintf(cvalue, "%d", value);
				param_values[col] = strdup(cvalue);
				free_list[col] = 1;
			}
			col++;
		} else {
			mdb_set_error(query->conn, MDB_ERR_OTHER, "Invalid format string.");
			retval = -1;
			goto err;
		}
	}
	param_count = col;

	res = PQexecParams(CONN(query->conn)->pg, query->sql, param_count, NULL,
			   (const char* const*)param_values, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) {
		int code = pgsql_convert_error(PQresultErrorField(res, PG_DIAG_SQLSTATE));
		mdb_set_error(query->conn, code, PQresultErrorMessage(res));
		retval = -1;
	}

	if (QUERY(query)->pg_res != NULL)
		PQclear(QUERY(query)->pg_res);
	QUERY(query)->pg_res = res;
  
err:
	for (i = 0; i < param_count; i++)
		if (free_list[i])
			free(param_values[i]);
	free(param_values);
	free(free_list);

	return retval;
}

static int pgsql_mdb_query_get_rows(mdb_query* query)
{
	PGresult* res = QUERY(query)->pg_res;
	if (res)
		return PQntuples(res);
	return -1;
}

static int pgsql_mdb_query_get_affect_rows(mdb_query* query)
{
	PGresult* res = QUERY(query)->pg_res;
	if (res)
		return atoi(PQcmdTuples(res));
	return -1;
}

static int pgsql_mdb_query_get_last_id(mdb_query* query, const char* seq_name)
{
	mdb_set_error(query->conn, MDB_ERR_OTHER, "pgsql_mdb_query_get_last_id() is not implemented!");
	return -1;
}

mdb_driver pgsql_driver =
{
	.name = "pgsql",
	.connect = pgsql_mdb_connect,
	.disconnect = pgsql_mdb_disconnect,
	.begin = pgsql_mdb_begin,
	.commit = pgsql_mdb_commit,
	.rollback = pgsql_mdb_rollback,
	.query_new = pgsql_mdb_query_new,
	.query_fill = pgsql_mdb_query_fill,
	.query_free = pgsql_mdb_query_free,
	.query_getv = pgsql_mdb_query_getv,
	.query_geta = pgsql_mdb_query_geta,
	.query_putv = pgsql_mdb_query_putv,
	.query_get_rows = pgsql_mdb_query_get_rows,
	.query_get_affect_rows = pgsql_mdb_query_get_affect_rows,
	.query_get_last_id = pgsql_mdb_query_get_last_id,
};

#else
mdb_driver pgsql_driver = {};
#endif	/* DROP_PG */
