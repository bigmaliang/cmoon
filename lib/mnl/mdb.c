#include "mheads.h"

static mdb_driver* drivers[MDB_DV_NUM] = {
	&sqlite_driver,
	&pgsql_driver,
	&mysql_driver,
};

/*
 * function sets one 
 */
NEOERR* mdb_init(mdb_conn **conn, char *dsn)
{
	if (!dsn) return nerr_raise(NERR_ASSERT, "dns null");
	
	mdb_conn *lconn = NULL;
	NEOERR *err;

	mtc_dbg("connect to %s ...", dsn);
	*conn = NULL;

	for (int i = 0; i < MDB_DV_NUM; i++) {
		const char* name = drivers[i]->name;
		if (name && !strncmp(dsn, name, strlen(name)) && dsn[strlen(name)] == ':') {
			const char* drv_dsn = strchr(dsn, ':') + 1;
			err = drivers[i]->connect(drv_dsn, &lconn);
			if (err != STATUS_OK) return nerr_pass(err);
			lconn->dsn = strdup(drv_dsn);
			lconn->driver = drivers[i];
			break;
		}
	}

	*conn = lconn;

	return STATUS_OK;
}

void mdb_destroy(mdb_conn *conn)
{
	if (conn == NULL) return;
	conn->driver->disconnect(conn);
	if (conn->dsn) free(conn->dsn);
	if (conn->sql) free(conn->sql);
	free(conn);
}

const char* mdb_get_backend(mdb_conn* conn)
{
	if (conn == NULL) return NULL;
	return conn->driver->name;
}

NEOERR* mdb_begin(mdb_conn* conn)
{
	if (!conn) return nerr_raise(NERR_ASSERT, "conn null");
	NEOERR *err = conn->driver->begin(conn);
	if (err != STATUS_OK) return nerr_pass(err);
	conn->in_transaction = true;

	return STATUS_OK;
}

NEOERR* mdb_commit(mdb_conn* conn)
{
	if (!conn) return nerr_raise(NERR_ASSERT, "conn null");
	NEOERR *err = conn->driver->commit(conn);
	if (err != STATUS_OK) return nerr_pass(err);
	conn->in_transaction = false;

	return STATUS_OK;
}

NEOERR* mdb_rollback(mdb_conn* conn)
{
	if (!conn) return nerr_raise(NERR_ASSERT, "conn null");
	NEOERR *err = conn->driver->rollback(conn);
	if (err != STATUS_OK) return nerr_pass(err);
	conn->in_transaction = false;
	
	return STATUS_OK;
}

NEOERR* mdb_finish(mdb_conn* conn)
{
	if (!conn) return nerr_raise(NERR_ASSERT, "conn null");
	if (!conn->in_transaction) return nerr_raise(NERR_ASSERT, "not in transaction");

	NEOERR *err = mdb_commit(conn);
	if (err != STATUS_OK) {
		mdb_rollback(conn);
		return nerr_pass(err);
	}

	return STATUS_OK;
}

/*
 * function sets two
 */
NEOERR* mdb_exec(mdb_conn* conn, int *affectrow, const char* sql_fmt,
				 const char* fmt, ...)
{
	if (!conn || !sql_fmt) return nerr_raise(NERR_ASSERT, "param error");

	va_list ap;
	char *p;
	bool needesc = false;
	char *sqlstr;
	NEOERR *err;
	
	p = (char*)sql_fmt;
	while (*p != '\0') {
		if (*p == '%' && *p+1 != '%' && *p-1 != '%') {
			needesc = true;
		}
		p++;
	}

	va_start(ap, fmt);
	if (needesc) {
		sqlstr = vsprintf_alloc(sql_fmt, ap);
		if (!sqlstr) return nerr_raise(NERR_NOMEM, "alloc sql string fail");
		err = conn->driver->query_fill(conn, sqlstr);
		if (err != STATUS_OK) {
			free(sqlstr);
			return nerr_pass(err);
		}
		/*
		 * vsprintf_allc() and vsnprintf() both not modify the ap
		 * so, we need to do this...
		 */
		p = (char*)sql_fmt;
		while (*p != '\0') {
			if (*p == '%' && *p+1 != '%' && *p-1 != '%')
				va_arg(ap, void);
			p++;
		}
	} else {
		err = conn->driver->query_fill(conn, sql_fmt);
		if (err != STATUS_OK) return nerr_pass(err);
	}

	err = conn->driver->query_putv(conn, fmt, ap);
	va_end(ap);

	if (affectrow != NULL)
		*affectrow = mdb_get_affect_rows(conn);

	return nerr_pass(err);
}

NEOERR* mdb_put(mdb_conn* conn, const char* fmt, ...)
{
	if (!conn || !fmt) return nerr_raise(NERR_ASSERT, "param error");
  
	va_list ap;
	va_start(ap, fmt);
	NEOERR *err = conn->driver->query_putv(conn, fmt, ap);
	va_end(ap);

	return nerr_pass(err);
}

NEOERR* mdb_get(mdb_conn* conn, const char* fmt, ...)
{
	if (!conn || !fmt) return nerr_raise(NERR_ASSERT, "param error");
  
	va_list ap;
	va_start(ap, fmt);
	NEOERR *err = conn->driver->query_getv(conn, fmt, ap);
	va_end(ap);

	return nerr_pass(err);
}

NEOERR* mdb_geta(mdb_conn* conn, const char* fmt, char* res[])
{
	if (!conn || !fmt) return nerr_raise(NERR_ASSERT, "param error");

	return nerr_pass(conn->driver->query_geta(conn, fmt, res));
}

NEOERR* mdb_set_row(HDF *hdf, mdb_conn* conn, char *cols, char *prefix)
{
	if (!conn || !hdf) return nerr_raise(NERR_ASSERT, "param error");

	int qrcnt, i;
	char qrarray[QR_NUM_MAX][LEN_ST];
	char *col[QR_NUM_MAX];
	char fmt[LEN_ST] = {0};
	NEOERR *err;
	
	memset(fmt, 0x0, sizeof(fmt));
	memset(qrarray, 0x0, sizeof(qrarray));
	
	mmisc_set_qrarray(cols, qrarray, &qrcnt);
	memset(fmt, 's', qrcnt);

	err = mdb_geta(conn, fmt, col);
	if (err != STATUS_OK) return nerr_pass(err);

	for (i = 0; i < qrcnt; i++) {
		/* TODO cols NULL means what? see mdb_set_rows() */
		if (prefix) {
			err = hdf_set_valuef(hdf, "%s.%s=%s", prefix, qrarray[i], col[i]);
			if (err != STATUS_OK) return nerr_pass(err);
		} else {
			hdf_set_valuef(hdf, "%s=%s", qrarray[i], col[i]);
			if (err != STATUS_OK) return nerr_pass(err);
		}
	}

	return STATUS_OK;
}

#define BUILD_HDF_FMT()													\
	do {																\
		memset(hdfkey, 0x0, sizeof(hdfkey));							\
		if (prefix) snprintf(hdfkey, sizeof(hdfkey), "%s.", prefix);	\
		if (keycol >= 0) {												\
			strncat(hdfkey, col[keycol], sizeof(hdfkey));				\
		} else {														\
			snprintf(tok, sizeof(tok), "%d", rowsn);					\
			strncat(hdfkey, tok, sizeof(hdfkey));						\
		}																\
		if (cols) {														\
			strcat(hdfkey, ".");										\
			strncat(hdfkey, qrarray[i], sizeof(hdfkey));				\
		}																\
		strcat(hdfkey, "=%s");											\
	} while (0)

NEOERR* mdb_set_rows(HDF *hdf, mdb_conn* conn, char *cols,
					 char *prefix, int keycol)
{
	if (!conn || !hdf) return nerr_raise(NERR_ASSERT, "param error");

	int qrcnt = 1, i;
	char qrarray[QR_NUM_MAX][LEN_ST];
	char *col[QR_NUM_MAX];
	char fmt[LEN_ST] = {0}, hdfkey[LEN_HDF_KEY] = {0}, tok[LEN_ST];
	NEOERR *err;
	
	memset(fmt, 0x0, sizeof(fmt));
	memset(qrarray, 0x0, sizeof(qrarray));

	if (cols) mmisc_set_qrarray(cols, qrarray, &qrcnt);
	memset(fmt, 's', qrcnt);
	if (keycol > qrcnt) keycol = 0;

	/* append to last child */
	int rowsn = 0;
	if (prefix) {
		HDF *res = hdf_get_child(hdf, prefix);
		while (res != NULL) {
			rowsn++;
			res = hdf_obj_next(res);
		}
	}

	while ( (err = mdb_geta(conn, fmt, col)) == STATUS_OK ){
		for (i = 0; i < qrcnt; i++) {
			BUILD_HDF_FMT();
			hdf_set_valuef(hdf, hdfkey, col[i]);
		}
		rowsn++;
	}
	nerr_ignore(&err);

	return STATUS_OK;
}

int mdb_get_rows(mdb_conn* conn)
{
	if (!conn) return -1;
  
	return conn->driver->query_get_rows(conn);
}

int mdb_get_affect_rows(mdb_conn* conn)
{
	if (!conn) return -1;
  
	return conn->driver->query_get_affect_rows(conn);
}

int mdb_get_last_id(mdb_conn* conn, const char* seq_name)
{
	if (!conn) return -1;

	return conn->driver->query_get_last_id(conn, seq_name);
}
