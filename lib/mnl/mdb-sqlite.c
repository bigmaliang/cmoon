#include "mdb-priv.h"
#include "mheads.h"

#ifndef DROP_SQLITE
#include <sqlite3.h>

struct _mdb_conn_sqlite
{
	mdb_conn base;
	sqlite3* handle;
};

enum _sqlite_query_state
{
	QUERY_STATE_INIT,          // just after sqlite3_prepare
	QUERY_STATE_ROW_PENDING,   // sqlite3_step was called and row was not retrieved using getv
	QUERY_STATE_ROW_READ,      // row was retrieved using getv
	QUERY_STATE_COMPLETED,     // no more rows
};

struct _mdb_query_sqlite
{
	mdb_query base;
	sqlite3_stmt* stmt;
	int state;
};

#define CONN(c) ((struct _mdb_conn_sqlite*)(c))
#define QUERY(c) ((struct _mdb_query_sqlite*)(c))

// dsn is filename
static mdb_conn* sqlite_mdb_connect(const char* dsn)
{
	struct _mdb_conn_sqlite* conn;

	conn = calloc(1, sizeof(struct _mdb_conn_sqlite));
	if (sqlite3_open(dsn, &conn->handle) == SQLITE_OK)
		sqlite3_busy_timeout(conn->handle, 30000);
	else
		mdb_set_error((mdb_conn*)conn, MDB_ERR_OTHER, sqlite3_errmsg(conn->handle));
	//g_printerr("GSQLW: [%p:%p] open\n", g_thread_self(), CONN(conn)->handle);
	return (mdb_conn*)conn;
}

static void sqlite_mdb_disconnect(mdb_conn* conn)
{
	if (sqlite3_close(CONN(conn)->handle) != SQLITE_OK)
		mtc_err("GSQLW: Can't close sqlite: %s\n", sqlite3_errmsg(CONN(conn)->handle));
}

static int sqlite_mdb_begin(mdb_conn* conn)
{
	return mdb_exec(conn, NULL, "BEGIN TRANSACTION", NULL);
}

static int sqlite_mdb_commit(mdb_conn* conn)
{
	return mdb_exec(conn, NULL, "COMMIT TRANSACTION", NULL);
}

static int sqlite_mdb_rollback(mdb_conn* conn)
{
	return mdb_exec(conn, NULL, "ROLLBACK TRANSACTION", NULL);
}

char* _sqlite_fixup_sql(const char* str)
{
	char* tmp = strdup(str);
	bool in_string = false;
	int i;

	if (tmp == NULL)
		return NULL;

	for (i = 0; i < strlen(tmp); i++) {
		if (tmp[i] == '\'')
			in_string = !in_string;
		if (!in_string && tmp[i] == '$' && isdigit(tmp[i+1]))
			tmp[i] = '?';
	}

	return tmp;
}

static void sqlite_mdb_query_free(mdb_query* query);

static mdb_query* sqlite_mdb_query_new(mdb_conn* conn, const char* sql_string)
{
	struct _mdb_query_sqlite* query;
	int rs;
  
	query = calloc(1, sizeof(struct _mdb_query_sqlite));
	query->base.conn = conn;
	if (sql_string != NULL)
		query->base.sql = _sqlite_fixup_sql(sql_string);
	query->state = QUERY_STATE_INIT;

	if (query->base.sql != NULL) {
		rs = sqlite3_prepare_v2(CONN(conn)->handle, query->base.sql, -1, &query->stmt, NULL);
		//g_printerr("GSQLW: [%p:%p:%p]prepare: %s\n", g_thread_self(), CONN(conn)->handle, query->stmt, sql_string);
		if (rs != SQLITE_OK) {
			mdb_set_error(conn, MDB_ERR_OTHER, sqlite3_errmsg(CONN(conn)->handle));
			sqlite_mdb_query_free((mdb_query*)query);
			return NULL;
		}
	}

	return (mdb_query*)query;
}

static int sqlite_mdb_query_fill(mdb_query* query, const char* sql_string)
{
	mdb_conn* conn = query->conn;
	int rs;
	
	if (query->sql)
		free(query->sql);
	query->sql = NULL;
	if (QUERY(query)->base.sql)
		free(QUERY(query)->base.sql);
	QUERY(query)->base.sql = NULL;
	if (sql_string != NULL)
		QUERY(query)->base.sql = _sqlite_fixup_sql(sql_string);

	/* TODO does this should be done in query_empty? */
	QUERY(query)->state = QUERY_STATE_INIT;
	if (QUERY(query)->stmt != NULL)	{
		//g_printerr("GSQLW: [%p:%p:%p]finalize: %s\n", g_thread_self(), CONN(query->conn)->handle, QUERY(query)->stmt, query->sql);
		if (sqlite3_finalize(QUERY(query)->stmt) != SQLITE_OK)
			mtc_err("GSQLW: Can't close sqlite query: %s\n", sqlite3_errmsg(CONN(query->conn)->handle));
	}

	/*
	 * we haven't alloc another query,
	 * so, we don't need to append to queries ulist 
	 */
	//uListAppend(CONN(conn)->queries, query);

	if (QUERY(query)->base.sql != NULL) {
		rs = sqlite3_prepare_v2(CONN(conn)->handle, QUERY(query)->base.sql, -1, &QUERY(query)->stmt, NULL);
		//g_printerr("GSQLW: [%p:%p:%p]prepare: %s\n", g_thread_self(), CONN(conn)->handle, query->stmt, sql_string);
		if (rs != SQLITE_OK) {
			mdb_set_error(conn, MDB_ERR_OTHER, sqlite3_errmsg(CONN(conn)->handle));
			/* TODO WHY FREE ME? */
			//sqlite_mdb_query_free(query);
			return -1;
		}
	}
	return 0;
}

static void sqlite_mdb_query_free(mdb_query* query)
{
	if (QUERY(query)->stmt != NULL)	{
		//g_printerr("GSQLW: [%p:%p:%p]finalize: %s\n", g_thread_self(), CONN(query->conn)->handle, QUERY(query)->stmt, query->sql);
		if (sqlite3_finalize(QUERY(query)->stmt) != SQLITE_OK)
			mtc_err("GSQLW: Can't close sqlite query: %s\n", sqlite3_errmsg(CONN(query->conn)->handle));
		else
			QUERY(query)->stmt = NULL;
	}
	free(query->sql);
	free(query);
}

#if 0
static void sqlite_mdb_query_empty(mdb_query* query)
{
	/*
	 * we just want to free the result to excute another sql command, but,
	 * 1, freed the queries ulist means free the conn->query here,
	 * 2, finalized the stmt also can't do futher sql,
	 * so, it will be core dump.
	 * this toke me 5Hours(2009-01-20 21:00 ~ 2009-01-21 01:50), oh my god......
	 */
	//uListDestroy(&(CONN(query->conn)->queries), 0);

	/* TODO we need to free the dirty query's result for further sql exec */
	//if (QUERY(query)->stmt != NULL)	{
	//	//g_printerr("GSQLW: [%p:%p:%p]finalize: %s\n", g_thread_self(), CONN(query->conn)->handle, QUERY(query)->stmt, query->sql);
	//	if (sqlite3_finalize(QUERY(query)->stmt) != SQLITE_OK)
	//		mtc_err("GSQLW: Can't close sqlite query: %s\n", sqlite3_errmsg(CONN(query->conn)->handle));
	//}
}
#endif

static int sqlite_mdb_query_getv(mdb_query* query, const char* fmt, va_list ap)
{
	sqlite3_stmt* stmt = QUERY(query)->stmt;
	int param_count = fmt != NULL ? strlen(fmt) : 0;
	int i, rs;
	int col = 0;

	switch (QUERY(query)->state) {
	case QUERY_STATE_INIT: 
		mdb_set_error(query->conn, MDB_ERR_OTHER, "Invalid API use, call mdb_query_put() before mdb_query_get().");
		return -1;
	case QUERY_STATE_COMPLETED:
		//mdb_set_error(query->conn, MDB_ERR_RESULT_ENDED, "last row has fetched");
		return 1;
	case QUERY_STATE_ROW_READ:
		// fetch next row
		rs = sqlite3_step(stmt);
		if (rs == SQLITE_ROW)
			break;
		else if (rs == SQLITE_DONE) {
			//mdb_set_error(query->conn, MDB_ERR_RESULT_ENDED, "last row has fetched");
			QUERY(query)->state = QUERY_STATE_COMPLETED;
			return 1;
		}
		else
		{
			//XXX: set error based on sqlite state
			mdb_set_error(query->conn, MDB_ERR_OTHER, sqlite3_errmsg(CONN(query->conn)->handle));
			return -1;
		}
	case QUERY_STATE_ROW_PENDING:
		QUERY(query)->state = QUERY_STATE_ROW_READ;
		break;
	}

	for (i = 0; i < param_count; i++) {
		if (fmt[i] == 's') {
			char** str_ptr = (char**)va_arg(ap, char**);
			*str_ptr = (char*)sqlite3_column_text(stmt, col);
			col++;
		} else if (fmt[i] == 'S') {
			char** str_ptr = (char**)va_arg(ap, char**);
			*str_ptr = strdup((char*)sqlite3_column_text(stmt, col));
			col++;
		} else if (fmt[i] == 'i') {
			int* int_ptr = (int*)va_arg(ap, int*);
			*int_ptr = sqlite3_column_int(stmt, col);
			col++;
		} else if (fmt[i] == '?') { // null flag
			int* int_ptr = (int*)va_arg(ap, int*);
			*int_ptr = sqlite3_value_type(sqlite3_column_value(stmt, col)) == SQLITE_NULL;
		} else {
			mdb_set_error(query->conn, MDB_ERR_OTHER, "Invalid format string.");
			va_end(ap);
			return -1;
		}
	}

	return 0;
}

static int sqlite_mdb_query_geta(mdb_query* query, const char* fmt, char *r[])
{
	sqlite3_stmt* stmt = QUERY(query)->stmt;
	int param_count = fmt != NULL ? strlen(fmt) : 0;
	int i, rs;
	int col = 0;

	switch (QUERY(query)->state) {
	case QUERY_STATE_INIT: 
		mdb_set_error(query->conn, MDB_ERR_OTHER, "Invalid API use, call mdb_query_put() before mdb_query_get().");
		return -1;
	case QUERY_STATE_COMPLETED:
		//mdb_set_error(query->conn, MDB_ERR_RESULT_ENDED, "last row has fetched");
		return 1;
	case QUERY_STATE_ROW_READ:
		// fetch next row
		rs = sqlite3_step(stmt);
		if (rs == SQLITE_ROW)
			break;
		else if (rs == SQLITE_DONE) {
			//mdb_set_error(query->conn, MDB_ERR_RESULT_ENDED, "last row has fetched");
			QUERY(query)->state = QUERY_STATE_COMPLETED;
			return 1;
		}
		else
		{
			//XXX: set error based on sqlite state
			mdb_set_error(query->conn, MDB_ERR_OTHER, sqlite3_errmsg(CONN(query->conn)->handle));
			return -1;
		}
	case QUERY_STATE_ROW_PENDING:
		QUERY(query)->state = QUERY_STATE_ROW_READ;
		break;
	}

	for (i = 0; i < param_count; i++) {
		if (fmt[i] == 's') {
			r[i] = (char*)sqlite3_column_text(stmt, col);
		} else if (fmt[i] == 'S') {
			r[i] = strdup((char*)sqlite3_column_text(stmt, col));
		}
		col++;
	}

	return 0;
}

static int sqlite_mdb_query_putv(mdb_query* query, const char* fmt, va_list ap)
{
	sqlite3_stmt* stmt = QUERY(query)->stmt;
	int param_count = (fmt != NULL) ? strlen(fmt) : 0;
	int i, rs;
	int col = 1;

	if (QUERY(query)->state != QUERY_STATE_INIT) {
		if (sqlite3_reset(stmt) != SQLITE_OK) {
			mdb_set_error(query->conn, MDB_ERR_OTHER, sqlite3_errmsg(CONN(query->conn)->handle));
			return -1;
		}
	}

	for (i = 0; i < param_count; i++) {
		int is_null = 0;

		if (fmt[i] == '?') {
			is_null = (int)va_arg(ap, int);
			if (is_null)
				sqlite3_bind_null(stmt, col);
			i++;
		}

		if (fmt[i] == 's') {
			char* value = (char*)va_arg(ap, char*);
			if (!is_null) {
				if (value == NULL)
					sqlite3_bind_null(stmt, col);
				else
					sqlite3_bind_text(stmt, col, value, -1, SQLITE_TRANSIENT);
			}
			col++;
		}
		else if (fmt[i] == 'i')	{
			int value = (int)va_arg(ap, int);
			if (!is_null)
				sqlite3_bind_int(stmt, col, value);
			col++;
		} else {
			mdb_set_error(query->conn, MDB_ERR_OTHER, "Invalid format string.");
			return -1;
		}
	}

	mtc_dbg("%s %d", query->sql, param_count);
	rs = sqlite3_step(stmt);
	if (rs == SQLITE_DONE)
		QUERY(query)->state = QUERY_STATE_COMPLETED;
	else if (rs == SQLITE_ROW)
		QUERY(query)->state = QUERY_STATE_ROW_PENDING;
	else {
		//XXX: set error based on sqlite state
		mdb_set_error(query->conn, MDB_ERR_OTHER, sqlite3_errmsg(CONN(query->conn)->handle));
		return -1;
	}

	return 0;
}

static int sqlite_mdb_query_get_rows(mdb_query* query)
{
	int rs;
	int count = 0;
	sqlite3_stmt* stmt = QUERY(query)->stmt;

	if (QUERY(query)->state == QUERY_STATE_INIT) {
		mdb_set_error(query->conn, MDB_ERR_OTHER, "Invalid API use, call mdb_query_put() before mdb_query_get_rows().");
		return -1;
	}

	if (sqlite3_reset(stmt) != SQLITE_OK) {
		mdb_set_error(query->conn, MDB_ERR_OTHER, sqlite3_errmsg(CONN(query->conn)->handle));
		return -1;
	}

	while (TRUE) {
		rs = sqlite3_step(stmt);
		if (rs == SQLITE_DONE)
			break;
		else if (rs == SQLITE_ROW)
			count++;
		else {
			//XXX: set error based on sqlite state
			mdb_set_error(query->conn, MDB_ERR_OTHER, sqlite3_errmsg(CONN(query->conn)->handle));
			return -1;
		}
	}

	if (sqlite3_reset(stmt) != SQLITE_OK) {
		mdb_set_error(query->conn, MDB_ERR_OTHER, sqlite3_errmsg(CONN(query->conn)->handle));
		return -1;
	}

	rs = sqlite3_step(stmt);
	if (rs == SQLITE_ROW)
		QUERY(query)->state = QUERY_STATE_ROW_PENDING;
	else if (rs == SQLITE_DONE)
		QUERY(query)->state = QUERY_STATE_COMPLETED;
	else {
		//XXX: set error based on sqlite state
		mdb_set_error(query->conn, MDB_ERR_OTHER, sqlite3_errmsg(CONN(query->conn)->handle));
		return -1;
	}

	return count;
}

static int sqlite_mdb_query_get_affect_rows(mdb_query* query)
{
	mdb_set_error(query->conn, MDB_ERR_OTHER, "sqlite_mdb_query_get_affect_rows() is not implemented!");
	return -1;
}

static int sqlite_mdb_query_get_last_id(mdb_query* query, const char* seq_name)
{
	int id;

	id = sqlite3_last_insert_rowid(CONN(query->conn)->handle);

	return id;
}

mdb_driver sqlite_driver =
{
	.name = "sqlite",
	.connect = sqlite_mdb_connect,
	.disconnect = sqlite_mdb_disconnect,
	.begin = sqlite_mdb_begin,
	.commit = sqlite_mdb_commit,
	.rollback = sqlite_mdb_rollback,
	.query_new = sqlite_mdb_query_new,
	.query_fill = sqlite_mdb_query_fill,
	.query_free = sqlite_mdb_query_free,
	.query_getv = sqlite_mdb_query_getv,
	.query_geta = sqlite_mdb_query_geta,
	.query_putv = sqlite_mdb_query_putv,
	.query_get_rows = sqlite_mdb_query_get_rows,
	.query_get_affect_rows = sqlite_mdb_query_get_affect_rows,
	.query_get_last_id = sqlite_mdb_query_get_last_id,
};

#else
mdb_driver sqlite_driver = {};
#endif	/* DROP_SQLITE */
