#include "mdb-priv.h"

static mdb_driver* drivers[MDB_DV_NUM] = {
	&sqlite_driver,
	&pgsql_driver,
	&mysql_driver,
};

mdb_conn* mdb_connect(const char* dsn)
{
	int i;
	mdb_conn* conn = NULL;
	mdb_query* query = NULL;

	if (dsn == NULL)
		return NULL;
  
	for (i = 0; i < MDB_DV_NUM; i++) {
		const char* name = drivers[i]->name;
		if (name && !strncmp(dsn, name, strlen(name)) && dsn[strlen(name)] == ':') {
			const char* drv_dsn = strchr(dsn, ':') + 1;
			conn = drivers[i]->connect(drv_dsn);
			if (conn) {
				conn->dsn = strdup(drv_dsn);
				conn->driver = drivers[i];
			}
			uListInit(&(conn->queries), 0, 0);
			query = mdb_query_new(conn, NULL);
			if (query != NULL) {
				uListAppend(conn->queries, query);
			}
			return conn;
		}
	}

	return NULL;
}
void mdb_disconnect(mdb_conn* conn)
{
	if (conn == NULL)
		return;
	uListDestroyFunc(&(conn->queries), mdb_query_free);
	CONN_DRIVER(conn)->disconnect(conn);
	mdb_clear_error(conn);
	free(conn->dsn);
	free(conn);
}

mdb_query* mdb_query_new(mdb_conn* conn, const char* sql_fmt, ...)
{
	CONN_RETURN_VAL_IF_INVALID(conn, NULL);
	char *sqlstr = NULL;
  
	if (sql_fmt != NULL && strcmp(sql_fmt, "")) {
		va_list ap;
		va_start(ap, sql_fmt);
		sqlstr = vsprintf_alloc(sql_fmt, ap);
		va_end(ap);
		if (sqlstr == NULL) {
			mdb_set_error(conn, MDB_ERR_MEMORY_ALLOC, "calloc for ms query new failure.");
			return NULL;
		}
	}
	mdb_query *res = CONN_DRIVER(conn)->query_new(conn, sqlstr);
	free(sqlstr);
	return res;
}

int mdb_query_fill(mdb_query* query, const char* sql_fmt, ...)
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
	char *sqlstr;
	va_list ap;
	int ret;
  
	va_start(ap, sql_fmt);
	sqlstr = vsprintf_alloc(sql_fmt, ap);
	va_end(ap);
	if (sqlstr == NULL) {
		mdb_set_error(query->conn, MDB_ERR_MEMORY_ALLOC, "calloc for ms query new failure.");
		return -1;
	}
	ret = CONN_DRIVER(query->conn)->query_fill(query, sqlstr);
	free(sqlstr);
	return ret;
}

void mdb_query_free(void *data)
{
	mdb_query *query = (mdb_query*)data;
	if (query)
		QUERY_DRIVER(query)->query_free(query);
}

int mdb_query_putv(mdb_query* query, const char* fmt, va_list ap)
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
	return QUERY_DRIVER(query)->query_putv(query, fmt, ap);
}

int mdb_query_getv(mdb_query* query, const char* fmt, va_list ap)
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
	return QUERY_DRIVER(query)->query_getv(query, fmt, ap);
}

int mdb_query_geta(mdb_query* query, const char* fmt, char* res[])
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
	return QUERY_DRIVER(query)->query_geta(query, fmt, res);
}

int mdb_query_put(mdb_query* query, const char* fmt, ...)
{
	int retval;
	va_list ap;

	va_start(ap, fmt);
	retval = mdb_query_putv(query, fmt, ap);
	va_end(ap);

	return retval;
}

int mdb_query_get(mdb_query* query, const char* fmt, ...)
{
	int retval;
	va_list ap;

	va_start(ap, fmt);
	retval = mdb_query_getv(query, fmt, ap);
	va_end(ap);

	return retval;
}

int mdb_query_get_rows(mdb_query* query)
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
	return QUERY_DRIVER(query)->query_get_rows(query);
}

int mdb_query_get_affect_rows(mdb_query* query)
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
	return QUERY_DRIVER(query)->query_get_affect_rows(query);
}

int mdb_query_get_last_id(mdb_query* query, const char* seq_name)
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
	return QUERY_DRIVER(query)->query_get_last_id(query, seq_name);
}

