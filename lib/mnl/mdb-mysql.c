#include "mdb-priv.h"

#ifndef DROP_MYSQL
#include <mysql.h>

struct _mdb_conn_mysql
{
	mdb_conn base;
	MYSQL *mysql;
};

struct _mdb_query_mysql
{
	mdb_query base;
	MYSQL_RES *res;
	MYSQL_ROW row;
	int row_no;
};

struct _mdb_conn_parameter_mysql
{
	char *ip;
	unsigned int port;
	char *name;
	char *user;
	char *pass;
};

#define CONN(c) ((struct _mdb_conn_mysql*)(c))
#define QUERY(c) ((struct _mdb_query_mysql*)(c))

static int mysql_get_token(const char *dsn, char *tok, char **s, char **e)
{
	if (!dsn || !tok || !s || !e) return 1;
	
	char *p = strstr(dsn, tok);
	char *start, *end;
	start = end = NULL;
	if (p) {
		start = strchr(p, '=');
		if (start) start += 1;
		while (*start && isspace(*start)) start++;
		end = start;
		while (*end && !isspace(*end)) end++;
	}

	if (!start || !end || start == end) return 1;
	*s = start;
	*e = end;
	return 0;
}
static struct _mdb_conn_parameter_mysql* mysql_mdb_parse_dsn(const char *dsn)
{
	if (!dsn) return NULL;
	
	struct _mdb_conn_parameter_mysql *info = calloc(1, sizeof(struct _mdb_conn_parameter_mysql));
	if (!info)
		return NULL;

	char *s, *e, intval[64];
	int ret;

	ret = mysql_get_token(dsn, "hostaddr", &s, &e);
	if (ret == 0) info->ip = strndup(s, e-s);

	ret = mysql_get_token(dsn, "port", &s, &e);
	if (ret == 0) {
		strncpy(intval, s, e-s);
		info->port = (unsigned int)atoi(intval);
	}

	ret = mysql_get_token(dsn, "dbname", &s, &e);
	if (ret == 0) info->name = strndup(s, e-s);

	ret = mysql_get_token(dsn, "user", &s, &e);
	if (ret == 0) info->user = strndup(s, e-s);

	ret = mysql_get_token(dsn, "password", &s, &e);
	if (ret == 0) info->pass = strndup(s, e-s);
	
	return info;
}
static void mysql_mdb_free_conn_parameter(struct _mdb_conn_parameter_mysql *pa)
{
	if (!pa) return;
	
	if (pa->ip) free(pa->ip);
	if (pa->user) free(pa->user);
	if (pa->name) free(pa->name);
	if (pa->pass) free(pa->pass);
	free(pa);
}

static mdb_conn* mysql_mdb_connect(const char* dsn)
{
	struct _mdb_conn_mysql* conn;
  
	conn = calloc(1, sizeof(struct _mdb_conn_mysql));
	if (conn == NULL) {
		mtc_err("calloc for ms connect failure.");
		return NULL;
	}
	conn->mysql = mysql_init(NULL);
	if (conn->mysql == NULL) {
		mdb_set_error((mdb_conn*)conn, MDB_ERR_MEMORY_ALLOC, "calloc for query new failure.");
	} else {
		struct _mdb_conn_parameter_mysql *pa = mysql_mdb_parse_dsn(dsn);
		if (!pa) {
			mdb_set_error((mdb_conn*)conn, MDB_ERR_OTHER, mysql_error(conn->mysql));
			return (mdb_conn*)conn;
		}
		
		my_bool reconnect = 1;
		mysql_options(conn->mysql, MYSQL_OPT_RECONNECT, &reconnect);
		mysql_options(conn->mysql, MYSQL_SET_CHARSET_NAME, "utf8");
		mysql_options(conn->mysql, MYSQL_INIT_COMMAND, "SET NAMES 'utf8'");
		conn->mysql = mysql_real_connect(conn->mysql, pa->ip, pa->user, pa->pass, pa->name, 0, NULL, 0);
		mysql_mdb_free_conn_parameter(pa);
		if (conn->mysql == NULL) {
			mdb_set_error((mdb_conn*)conn, MDB_ERR_OTHER, mysql_error(conn->mysql));
		}
	}

	return (mdb_conn*)conn;
}

static void mysql_mdb_disconnect(mdb_conn* conn)
{
	if (CONN(conn)->mysql != NULL)
		mysql_close(CONN(conn)->mysql);
	CONN(conn)->mysql = NULL;
}

static int mysql_mdb_begin(mdb_conn* conn)
{
	int ret;
  
	ret = mysql_real_query(CONN(conn)->mysql, "BEGIN", strlen("BEGIN"));
	if (ret != 0) {
		mdb_set_error(conn, MDB_ERR_OTHER, mysql_error(CONN(conn)->mysql));
		return MDB_ERR_OTHER;
	}

	/* TODO free query? */
	return MDB_ERR_NONE;
}

static int mysql_mdb_commit(mdb_conn* conn)
{
	int ret;
  
	//ret = mysql_real_query(CONN(conn)->mysql, "COMMIT");
	ret = mysql_commit(CONN(conn)->mysql);
	if (ret != 0) {
		mdb_set_error(conn, MDB_ERR_OTHER, mysql_error(CONN(conn)->mysql));
		return MDB_ERR_OTHER;
	}
	
	return MDB_ERR_NONE;
}

static int mysql_mdb_rollback(mdb_conn* conn)
{
	int ret;
  
	//ret = mysql_real_query(CONN(conn)->mysql, "ROOLBACK");
	ret = mysql_rollback(CONN(conn)->mysql);
	if (ret != 0) {
		mdb_set_error(conn, MDB_ERR_OTHER, mysql_error(CONN(conn)->mysql));
		return MDB_ERR_OTHER;
	}
	
	return MDB_ERR_NONE;
}

static mdb_query* mysql_mdb_query_new(mdb_conn* conn, const char* sql_string)
{
	struct _mdb_query_mysql* query;
  
	query = calloc(1, sizeof(struct _mdb_query_mysql));
	if (query == NULL) {
		mdb_set_error(conn, MDB_ERR_MEMORY_ALLOC, "calloc for query new failure.");
		return NULL;
	}
	query->base.conn = conn;
	if (sql_string)
		query->base.sql = strdup(sql_string);

	return (mdb_query*)query;
}

static int mysql_mdb_query_fill(mdb_query* query, const char* sql_string)
{
	if (QUERY(query)->res != NULL)
		mysql_free_result(QUERY(query)->res);
	QUERY(query)->res = NULL;
	QUERY(query)->row_no = 0;
	
	if (query->sql)
		free(query->sql);
	query->sql = NULL;
	if (sql_string)
		query->sql = strdup(sql_string);

	return 0;
}

static void mysql_mdb_query_free(mdb_query* query)
{
	/*
	 * if (p)
	 * if (p != NULL)
	 * both can judge a pointer valid, which one better?
	 */
	if (QUERY(query)->res != NULL)
		mysql_free_result(QUERY(query)->res);
	QUERY(query)->res = NULL;
	if (query->sql)
		free(query->sql);
	query->sql = NULL;
	free(query);
}

static int mysql_mdb_query_getv(mdb_query* query, const char* fmt, va_list ap)
{
	MYSQL_RES* res = QUERY(query)->res;
	MYSQL_ROW row = QUERY(query)->row;
	int row_no = QUERY(query)->row_no;

	if (res == NULL)
		return -1;

	if (row_no >= mdb_query_get_rows(query)) {
		if (row_no == 0) {
			mdb_set_error(query->conn, MDB_ERR_NORESULT, "empty result");
			return MDB_ERR_NORESULT;
		}
		mdb_set_error(query->conn, MDB_ERR_RESULT_ENDED, "last row has fetched");
		return MDB_ERR_RESULT_ENDED;
	}

	int param_count = fmt != NULL ? strlen(fmt) : 0;
	int i, col = 0;

	row = mysql_fetch_row(res);
	if (row == NULL) {
		mdb_set_error(query->conn, MDB_ERR_RESULT_ENDED, "last row has fetched");
		return MDB_ERR_RESULT_ENDED;
	}
	if (param_count > mysql_num_fields(res)) {
		mdb_set_error(query->conn, MDB_ERR_RESULT_ENDED, "result col num failure");
		return MDB_ERR_RESULTE;
	}
	for (i = 0; i < param_count; i++) {
		if (fmt[i] == 's') {
			char** str_ptr = (char**)va_arg(ap, char**);
			if (row[col] == NULL)
				*str_ptr = NULL;
			else
				*str_ptr = row[col];
			col++;
		} else if (fmt[i] == 'S')	{
			char** str_ptr = (char**)va_arg(ap, char**);
			if (row[col] == NULL)
				*str_ptr = NULL;
			else
				*str_ptr = strdup(row[col]);
			col++;
		} else if (fmt[i] == 'i')	{
			int* int_ptr = (int*)va_arg(ap, int*);
			if (row[col] != NULL)
				*int_ptr = atoi(row[col]);
			col++;
		} else if (fmt[i] == '?')	{ // null flag
			int* int_ptr = (int*)va_arg(ap, int*);
			if (row[col] == NULL)
				*int_ptr = 1;
			else
				*int_ptr = 0;
		} else {
			mdb_set_error(query->conn, MDB_ERR_INPUTE, "Invalid format string.");
			va_end(ap);
			return MDB_ERR_INPUTE;
		}
	}

	QUERY(query)->row_no++;
	return MDB_ERR_NONE;
}

static int mysql_mdb_query_geta(mdb_query* query, const char* fmt, char* r[])
{
	MYSQL_RES* res = QUERY(query)->res;
	MYSQL_ROW row = QUERY(query)->row;
	int row_no = QUERY(query)->row_no;

	if (res == NULL)
		return -1;

	if (row_no >= mdb_query_get_rows(query)) {
		if (row_no == 0) {
			mdb_set_error(query->conn, MDB_ERR_NORESULT, "empty result");
			return MDB_ERR_NORESULT;
		}
		mdb_set_error(query->conn, MDB_ERR_RESULT_ENDED, "last row has fetched");
		return MDB_ERR_RESULT_ENDED;
	}

	int param_count = fmt != NULL ? strlen(fmt) : 0;
	int i, col = 0;

	row = mysql_fetch_row(res);
	if (row == NULL) {
		mdb_set_error(query->conn, MDB_ERR_RESULT_ENDED, "last row has fetched");
		return MDB_ERR_RESULT_ENDED;
	}
	if (param_count > mysql_num_fields(res)) {
		mdb_set_error(query->conn, MDB_ERR_RESULT_ENDED, "result col num failure");
		return MDB_ERR_RESULTE;
	}
	for (i = 0; i < param_count; i++) {
		if (fmt[i] == 's') {
			if (row[col] == NULL)
				r[i] = NULL;
			else
				r[i] = row[col];
		} else if (fmt[i] == 'S')	{
			if (row[col] == NULL)
				r[i] = NULL;
			else
				r[i] = strdup(row[col]);
		}
		col++;
	}
	
	QUERY(query)->row_no++;
	return MDB_ERR_NONE;
}

static int mysql_add_sql_int(char **sql, int val)
{
	char tok[64];
	char *ostr = *sql;
	char *p, *q;
	
	p = strchr(ostr, '$');
	if (p != NULL) {
		q = p+1;
		while (q != NULL && *q != '\0' && isdigit(*q)) {
			q++;
		}
	}
	if (!p || !q) return 1;
	
	sprintf(tok, "%d", val);
	*sql = calloc(1, strlen(ostr)+strlen(tok)+1);
	if (*sql == NULL) return 2;
	char *nstr = *sql;
	strncpy(nstr, ostr, p-ostr);
	strcat(nstr, tok);
	strcat(nstr, q);

	free(ostr);
	return 0;
}

static int mysql_add_sql_str(MYSQL *mysql, char **sql, char *val)
{
	char *ostr = *sql;
	char *p, *q;
	
	p = strchr(ostr, '$');
	if (p != NULL) {
		q = p+1;
		while (q != NULL && *q != '\0' && isdigit(*q)) {
			q++;
		}
	}
	if (!p || !q) return 1;
	
	*sql = calloc(1, strlen(ostr)+strlen(val)*2+2);
	if (*sql == NULL) return 2;
	char *nstr = *sql;
	strncpy(nstr, ostr, p-ostr);
	mysql_real_escape_string(mysql, nstr+(p-ostr), val, strlen(val));
	strcat(nstr, q);

	free(ostr);
	return 0;
}

static int mysql_mdb_query_putv(mdb_query* query, const char* fmt, va_list ap)
{
	int param_count = (fmt != NULL) ? strlen(fmt) : 0;
	char** param_values = calloc(param_count, sizeof(char*));
	int* free_list = calloc(param_count, sizeof(int));
	if (param_values == NULL || free_list == NULL) {
		mdb_set_error(query->conn, MDB_ERR_MEMORY_ALLOC,
					  "calloc for query putv failure.");
		return MDB_ERR_MEMORY_ALLOC;
	}
	int i, col = 0, retval = MDB_ERR_NONE;

	int is_null;
	for (i = 0; i < param_count; i++) {
		is_null = 0;

		if (fmt[i] == '?') {
			is_null = (int)va_arg(ap, int);
			i++;
		}
		
		if (fmt[i] == 's') {
			char* value = (char*)va_arg(ap, char*);
			if (!is_null)
				if (mysql_add_sql_str(CONN(query->conn)->mysql,
									  &(query->sql), value) != 0) {
					mdb_set_error(query->conn, MDB_ERR_INPUTE,
								  "Invalid sql string.");
					retval = MDB_ERR_INPUTE;
					goto err;
				}
			col++;
		}
		else if (fmt[i] == 'i')	{
			int value = (int)va_arg(ap, int);
			if (!is_null) {
				if (mysql_add_sql_int(&(query->sql), value) != 0) {
					mdb_set_error(query->conn, MDB_ERR_INPUTE,
								  "Invalid sql string.");
					retval = MDB_ERR_INPUTE;
					goto err;
				}
			}
			col++;
		} else {
			mdb_set_error(query->conn, MDB_ERR_INPUTE, "Invalid format string.");
			retval = MDB_ERR_INPUTE;
			goto err;
		}
	}
	
	mtc_dbg("%s %d", query->sql, param_count);
	int ret = mysql_real_query(CONN(query->conn)->mysql, query->sql, strlen(query->sql));
	if (ret != 0) {
		mtc_err("%s %s", query->sql, mysql_error(CONN(query->conn)->mysql));
		mdb_set_error(query->conn, MDB_ERR_OTHER, mysql_error(CONN(query->conn)->mysql));
		retval = MDB_ERR_OTHER;
	}

	if (QUERY(query)->res != NULL)
		mysql_free_result(QUERY(query)->res);
	QUERY(query)->res = mysql_store_result(CONN(query->conn)->mysql);
	/* NEED res judgement? how do i use INSERT? */
	/*
	if (QUERY(query)->res == NULL) {
		mtc_err("%s %s", query->sql, mysql_error(CONN(query->conn)->mysql));
		mdb_set_error(query->conn, MDB_ERR_OTHER, mysql_error(CONN(query->conn)->mysql));
		retval = -3;
	}
	*/
  
err:
	for (i = 0; i < param_count; i++)
		if (free_list[i])
			free(param_values[i]);
	free(param_values);
	free(free_list);
	return retval;
}

static int mysql_mdb_query_get_rows(mdb_query* query)
{
	MYSQL_RES* res = QUERY(query)->res;
	if (res)
		return (int)mysql_num_rows(res);
	return -1;
}

static int mysql_mdb_query_get_affect_rows(mdb_query* query)
{
	return (int)mysql_affected_rows(CONN(query->conn)->mysql);
}

static int mysql_mdb_query_get_last_id(mdb_query* query, const char* seq_name)
{
	return (int)mysql_insert_id(CONN(query->conn)->mysql);
}

mdb_driver mysql_driver =
{
	.name = "mysql",
	.connect = mysql_mdb_connect,
	.disconnect = mysql_mdb_disconnect,
	.begin = mysql_mdb_begin,
	.commit = mysql_mdb_commit,
	.rollback = mysql_mdb_rollback,
	.query_new = mysql_mdb_query_new,
	.query_fill = mysql_mdb_query_fill,
	.query_free = mysql_mdb_query_free,
	.query_getv = mysql_mdb_query_getv,
	.query_geta = mysql_mdb_query_geta,
	.query_putv = mysql_mdb_query_putv,
	.query_get_rows = mysql_mdb_query_get_rows,
	.query_get_affect_rows = mysql_mdb_query_get_affect_rows,
	.query_get_last_id = mysql_mdb_query_get_last_id,
};

#else
mdb_driver mysql_driver = {};
#endif	/* DROP_MYSQL */
