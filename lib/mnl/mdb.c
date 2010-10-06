#include "mheads.h"
#include "mdb-priv.h"

/*
 * function sets one 
 */
int mdb_init(mdb_conn **conn, char *dsn)
{
	mdb_conn *lconn = NULL;
	if (dsn == NULL) {
		mtc_err("init db err. dsn is null");
		return RET_RBTOP_INITE;
	}
	mtc_dbg("connect to %s ...", dsn);
	lconn = mdb_connect(dsn);
	if (mdb_get_errcode(lconn) != MDB_ERR_NONE) {
		mtc_err("connect to %s failure %s", dsn, mdb_get_errmsg(lconn));
		*conn = NULL;
		mdb_destroy(lconn);
		return RET_RBTOP_INITE;
	}
	*conn = lconn;
	return RET_RBTOP_OK;
}

void mdb_destroy(mdb_conn *conn)
{
	if (conn == NULL)
		return;
	mdb_disconnect(conn);
}

const char* mdb_get_backend(mdb_conn* conn)
{
	if (conn == NULL)
		return "";
	return conn->driver->name;
}

const char* mdb_get_errmsg(mdb_conn* conn)
{
	if (conn == NULL)
		return "Connection obejct is NULL.";
	return conn->errmsg;
}

int mdb_get_errcode(mdb_conn* conn)
{
	if (conn == NULL)
		return MDB_ERR_OTHER;
	return conn->errcode;
}

void mdb_set_error(mdb_conn* conn, int code, const char* msg)
{
	CONN_RETURN_IF_INVALID(conn);
	conn->errcode = code;
	free(conn->errmsg);
	conn->errmsg = strdup(msg);
}

void mdb_clear_error(mdb_conn* conn)
{
	if (conn == NULL)
		return;
	conn->errcode = MDB_ERR_NONE;
	free(conn->errmsg);
	conn->errmsg = NULL;
}

int mdb_begin(mdb_conn* conn)
{
	CONN_RETURN_VAL_IF_INVALID(conn, -1);
	int retval = CONN_DRIVER(conn)->begin(conn);
	if (retval == 0)
		conn->in_transaction = true;
	return retval;
}

int mdb_commit(mdb_conn* conn)
{
	CONN_RETURN_VAL_IF_INVALID(conn, -1);
	int retval = CONN_DRIVER(conn)->commit(conn);
	if (retval == 0)
		conn->in_transaction = false;
	return retval;
}

int mdb_rollback(mdb_conn* conn)
{
	if (conn == NULL)
		return -1;
	int retval = CONN_DRIVER(conn)->rollback(conn);
	if (retval == 0)
		conn->in_transaction = false;
	return retval;
}

int mdb_finish(mdb_conn* conn)
{
	if (conn == NULL)
		return -1;
	if (!conn->in_transaction)
		return -1;
	if (mdb_get_errcode(conn) != MDB_ERR_NONE)
	{
		mdb_rollback(conn);
		return -1;
	}
	mdb_commit(conn);
	return 0;
}

/*
 * function sets two
 */
int mdb_exec(mdb_conn* conn, int *affectrow, const char* sql_fmt, const char* fmt, ...)
{
	CONN_RETURN_VAL_IF_INVALID(conn, -1);
	
	mdb_query *query;
	uListGet(conn->queries, 0, (void**)&query);
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
	
	int retval;
	va_list ap;
	char *p;
	bool needesc = false;
	char *sqlstr;
	
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
		if (sqlstr == NULL) {
			mdb_set_error(conn, MDB_ERR_MEMORY_ALLOC, "calloc for ms query new failure.");
			return -1;
		}
		/* use the first query in the connector->queries list default */
		mdb_query_fill(query, sqlstr);
		free(sqlstr);
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
		mdb_query_fill(query, sql_fmt);
	}

	retval = mdb_query_putv(query, fmt, ap);
	va_end(ap);

	if (affectrow != NULL)
		*affectrow = mdb_get_affect_rows(conn);

	return retval;
}

int mdb_put(mdb_conn* conn, const char* fmt, ...)
{
	CONN_RETURN_VAL_IF_INVALID(conn, -1);
	mdb_query *query;
	uListGet(conn->queries, 0, (void**)&query);
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
  
	int retval;
	va_list ap;

	va_start(ap, fmt);
	retval = mdb_query_putv(query, fmt, ap);
	va_end(ap);

	return retval;
}

int mdb_get(mdb_conn* conn, const char* fmt, ...)
{
	CONN_RETURN_VAL_IF_INVALID(conn, -1);
	mdb_query *query;
	uListGet(conn->queries, 0, (void**)&query);
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
  
	int retval;
	va_list ap;

	va_start(ap, fmt);
	retval = mdb_query_getv(query, fmt, ap);
	va_end(ap);

	return retval;
}

int mdb_geta(mdb_conn* conn, const char* fmt, char* res[])
{
	CONN_RETURN_VAL_IF_INVALID(conn, -1);
	mdb_query *query;
	uListGet(conn->queries, 0, (void**)&query);
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
  
	return mdb_query_geta(query, fmt, res);
}

int mdb_set_row(HDF *hdf, mdb_conn* conn, char *cols, char *prefix)
{
	int qrcnt, i;
	char qrarray[QR_NUM_MAX][LEN_ST];
	char *col[QR_NUM_MAX];
	char fmt[LEN_ST] = {0};
	int ret;
	
	memset(fmt, 0x0, sizeof(fmt));
	memset(qrarray, 0x0, sizeof(qrarray));
	
	mmisc_set_qrarray(cols, qrarray, &qrcnt);
	memset(fmt, 's', qrcnt);
	
	ret = mdb_geta(conn, fmt, col);
	if (ret != MDB_ERR_NONE) {
		mtc_err("db exec error %s", mdb_get_errmsg(conn));
		return ret;
	}
	
	for (i = 0; i < qrcnt; i++) {
		/* TODO cols NULL means what? see mdb_set_rows() */
		if (prefix)
			hdf_set_valuef(hdf, "%s.%s=%s", prefix, qrarray[i], col[i]);
		else
			hdf_set_valuef(hdf, "%s=%s", qrarray[i], col[i]);
	}

	return MDB_ERR_NONE;
}

#define BUILD_HDF_FMT()													\
	do {																\
		memset(hdfkey, 0x0, sizeof(hdfkey));							\
		if (prefix) snprintf(hdfkey, sizeof(hdfkey), "%s.", prefix);	\
		if (keycol > 0) {												\
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

int mdb_set_rows(HDF *hdf, mdb_conn* conn, char *cols,
				 char *prefix, int keycol)
{
	int qrcnt = 1, i;
	char qrarray[QR_NUM_MAX][LEN_ST];
	char *col[QR_NUM_MAX];
	char fmt[LEN_ST] = {0}, hdfkey[LEN_HDF_KEY] = {0}, tok[LEN_ST];
	int ret;
	
	memset(fmt, 0x0, sizeof(fmt));
	memset(qrarray, 0x0, sizeof(qrarray));

	if (cols) mmisc_set_qrarray(cols, qrarray, &qrcnt);
	memset(fmt, 's', qrcnt);
	if (keycol > qrcnt) keycol = 0;

	/* append to last child */
	int rowsn = 0;
	if (prefix)
		snprintf(hdfkey, sizeof(hdfkey), "%s.0", prefix);
	HDF *res = hdf_get_obj(hdf, hdfkey);
	while (res != NULL) {
		rowsn++;
		res = hdf_obj_next(res);
	}

	ret = mdb_get_errcode(conn);
	if (ret != MDB_ERR_NONE) {
		mtc_err("db exec error %s", mdb_get_errmsg(conn));
		return ret;
	}

	while (mdb_geta(conn, fmt, col) == MDB_ERR_NONE ){
		for (i = 0; i < qrcnt; i++) {
			BUILD_HDF_FMT();
			hdf_set_valuef(hdf, hdfkey, col[i]);
		}
		rowsn++;
	}

	return MDB_ERR_NONE;
}

int mdb_get_rows(mdb_conn* conn)
{
	CONN_RETURN_VAL_IF_INVALID(conn, -1);
	mdb_query *query;
	uListGet(conn->queries, 0, (void**)&query);
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
  
	return QUERY_DRIVER(query)->query_get_rows(query);
}

int mdb_get_affect_rows(mdb_conn* conn)
{
	CONN_RETURN_VAL_IF_INVALID(conn, -1);
	mdb_query *query;
	uListGet(conn->queries, 0, (void**)&query);
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
  
	return QUERY_DRIVER(query)->query_get_affect_rows(query);
}

int mdb_get_last_id(mdb_conn* conn, const char* seq_name)
{
	CONN_RETURN_VAL_IF_INVALID(conn, -1);
	mdb_query *query = (mdb_query*)(conn->queries->items[0]);
	QUERY_RETURN_VAL_IF_INVALID(query, -1);

	return QUERY_DRIVER(query)->query_get_last_id(query, seq_name);
}

/*
 * function sets three
 */
int mdb_exec_apart(mdb_conn* conn, mdb_query **pquery,
				   int *affectrow, const char* sql_fmt, const char* fmt, ...)
{
	CONN_RETURN_VAL_IF_INVALID(conn, -1);
	
	mdb_query *query = mdb_query_new(conn, NULL);
	if (query != NULL) {
		uListAppend(conn->queries, query);
		*pquery = query;
	}
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
	
	int retval;
	va_list ap;
	char *p;
	bool needesc;

	char *sqlstr;
	
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
		if (sqlstr == NULL) {
			mdb_set_error(conn, MDB_ERR_MEMORY_ALLOC, "calloc for ms query new failure.");
			return -1;
		}
		/* use the first query in the connector->queries list default */
		mdb_query_fill(query, sqlstr);
		free(sqlstr);
		/*
		 * vsprintf_allc() and vsnprintf() both not modify the ap
		 * so, we need to do this...
		 */
		char *p = (char*)sql_fmt;
		while (*p != '\0') {
			if (*p == '%' && *p+1 != '%' && *p-1 != '%')
				va_arg(ap, void);
			p++;
		}
	} else {
		mdb_query_fill(query, sql_fmt);
	}

	retval = mdb_query_putv(query, fmt, ap);
	va_end(ap);

	if (affectrow != NULL)
		*affectrow = mdb_get_affect_rows(conn);

	return retval;
}

int mdb_put_apart(mdb_query *query, const char* fmt, ...)
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
  
	int retval;
	va_list ap;

	va_start(ap, fmt);
	retval = mdb_query_putv(query, fmt, ap);
	va_end(ap);

	return retval;
}

int mdb_get_apart(mdb_query *query, const char* fmt, ...)
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
  
	int retval;
	va_list ap;

	va_start(ap, fmt);
	retval = mdb_query_getv(query, fmt, ap);
	va_end(ap);

	return retval;
}

int mdb_geta_apart(mdb_query *query, const char* fmt, char* res[])
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
  
	return mdb_query_geta(query, fmt, res);
}

int mdb_get_rows_apart(mdb_query *query)
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
  
	return QUERY_DRIVER(query)->query_get_rows(query);
}

int mdb_get_affect_rows_apart(mdb_query *query)
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);
  
	return QUERY_DRIVER(query)->query_get_affect_rows(query);
}

int mdb_get_last_id_apart(mdb_query *query, const char* seq_name)
{
	QUERY_RETURN_VAL_IF_INVALID(query, -1);

	return QUERY_DRIVER(query)->query_get_last_id(query, seq_name);
}

static void get_errmsg(int ret, char *res)
{
	switch (ret) {
	case RET_RBTOP_INITE:
		strcpy(res, "链接数据库错误");
		break;
	case RET_RBTOP_HDFNINIT:
		strcpy(res, "输入数据错误");
		break;
	case RET_RBTOP_INPUTE:
		strcpy(res, "输入参数错误");
		break;
	case RET_RBTOP_OPCODEE:
		strcpy(res, "操作码错误");
		break;
	case RET_RBTOP_DBNINIT:
		strcpy(res, "数据库未初始化");
		break;
	case RET_RBTOP_DBINTRANS:
		strcpy(res, "数据库操作未提交");
		break;
	case RET_RBTOP_SELECTE:
		strcpy(res, "数据库查询失败");
		break;
	case RET_RBTOP_UPDATEE:
		strcpy(res, "数据库更新失败");
		break;
	case RET_RBTOP_DELETEE:
		strcpy(res, "数据库删除失败");
		break;
	case RET_RBTOP_EVTNINIT:
		strcpy(res, "事件后台初始化失败");
		break;
	case RET_RBTOP_MEMALLOCE:
		strcpy(res, "分配内存失败");
		break;
	case RET_RBTOP_CREATEFE:
		strcpy(res, "创建文件失败");
		break;
	case RET_RBTOP_EVTE:
		strcpy(res, "事件后台处理失败");
		break;
	default:
		strcpy(res, "数据库操作错误");
		break;
	}
}

void mdb_opfinish(int ret, HDF *hdf, mdb_conn *conn,
				  char *target, char *url, bool header)
{
	char msg[LEN_SM];
	
	if (ret == RET_RBTOP_OK) {
		return;
	}

	get_errmsg(ret, msg);
	mutil_redirect(msg, target, url, header);

	/* conn destroy by user */
	/*
	if (conn != NULL) {
		mdb_destroy(conn);
	}
	*/
	/* TODO system resource need free*/
	exit(ret);
}

void mdb_opfinish_json(int ret, HDF *hdf, mdb_conn *conn)
{
	char msg[LEN_SM];
	
	if (ret == RET_RBTOP_OK) {
		hdf_set_value(hdf, PRE_SUCCESS, "1");
		return;
	}

	hdf_remove_tree(hdf, PRE_SUCCESS);
	if (!hdf_get_obj(hdf, PRE_ERRMSG)) {
		get_errmsg(ret, msg);
		hdf_set_value(hdf, PRE_ERRMSG, msg);
	}
	hdf_set_int_value(hdf, PRE_ERRCODE, ret);
	//mjson_output_hdf(hdf, 0);
	
	/* conn destroy by user */
	/*
	if (conn != NULL) {
		mdb_destroy(conn);
	}
	*/
	/* TODO system resource need free*/
	//exit(ret);
}
