#include "mheads.h"
#ifndef DROP_MYSQL
#include <mysql.h>

struct _mdb_conn_mysql
{
    mdb_conn base;
    MYSQL *mysql;
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

static NEOERR* mysql_mdb_connect(const char* dsn, mdb_conn **rconn)
{
    struct _mdb_conn_mysql* conn;

    *rconn = NULL;
    conn = calloc(1, sizeof(struct _mdb_conn_mysql));
    if (!conn) return nerr_raise(NERR_NOMEM, "calloc for ms connect failure");
    *rconn = (mdb_conn*)conn;

    conn->mysql = mysql_init(NULL);
    if (!conn->mysql) return nerr_raise(NERR_NOMEM, "calloc for mysql failure");

    struct _mdb_conn_parameter_mysql *pa = mysql_mdb_parse_dsn(dsn);
    if (!pa) return nerr_raise(NERR_ASSERT, "parse dsn %s failure", dsn);

    my_bool reconnect = 1;
    mysql_options(conn->mysql, MYSQL_OPT_RECONNECT, &reconnect);
    mysql_options(conn->mysql, MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(conn->mysql, MYSQL_INIT_COMMAND, "SET NAMES 'utf8'");
    if (!mysql_real_connect(conn->mysql, pa->ip, pa->user,
                            pa->pass, pa->name, 0, NULL, 0))
        return nerr_raise(NERR_SYSTEM, "connect to %s failure %s",
                          dsn, mysql_error(conn->mysql));

    mysql_mdb_free_conn_parameter(pa);

    return STATUS_OK;
}

static void mysql_mdb_disconnect(mdb_conn* conn)
{
    if (CONN(conn)->mysql != NULL)
        mysql_close(CONN(conn)->mysql);
    CONN(conn)->mysql = NULL;
    if (CONN(conn)->res != NULL)
        mysql_free_result(CONN(conn)->res);
    CONN(conn)->res = NULL;
}

static NEOERR* mysql_mdb_begin(mdb_conn* conn)
{
    if (mysql_real_query(CONN(conn)->mysql, "BEGIN", strlen("BEGIN")) != 0)
        return nerr_raise(NERR_DB, mysql_error(CONN(conn)->mysql));

    return STATUS_OK;
}

static NEOERR* mysql_mdb_commit(mdb_conn* conn)
{
    if (mysql_commit(CONN(conn)->mysql) != 0)
        return nerr_raise(NERR_DB, mysql_error(CONN(conn)->mysql));

    return STATUS_OK;
}

static NEOERR* mysql_mdb_rollback(mdb_conn* conn)
{
    if (mysql_rollback(CONN(conn)->mysql) != 0)
        return nerr_raise(NERR_DB, mysql_error(CONN(conn)->mysql));

    return STATUS_OK;
}

static NEOERR* mysql_mdb_query_fill(mdb_conn* conn, const char* sql_string)
{
    if (CONN(conn)->res != NULL)
        mysql_free_result(CONN(conn)->res);
    CONN(conn)->res = NULL;
    CONN(conn)->row_no = 0;

    if (conn->sql) free(conn->sql);
    conn->sql = NULL;
    if (sql_string)    conn->sql = strdup(sql_string);

    return STATUS_OK;
}

static NEOERR* mysql_mdb_query_getv(mdb_conn* conn, const char* fmt, va_list ap)
{
    MYSQL_RES* res = CONN(conn)->res;
    MYSQL_ROW row = CONN(conn)->row;
    int row_no = CONN(conn)->row_no;
    NEOERR *err = STATUS_OK;

    if (res == NULL) return nerr_raise(NERR_DB, "attemp fetch null res");

    if (row_no >= mdb_get_rows(conn)) {
        if (row_no == 0) return nerr_raise(NERR_NOT_FOUND, "empty result");
        return nerr_raise(NERR_OUTOFRANGE, "last row has fetched");
    }

    int param_count = fmt != NULL ? strlen(fmt) : 0;
    int i, col = 0;

    row = mysql_fetch_row(res);
    if (!row) return nerr_raise(NERR_OUTOFRANGE, "last row has fetched");
    if (param_count > mysql_num_fields(res))
        return nerr_raise(NERR_ASSERT, "result col num not match");

    for (i = 0; i < param_count; i++) {
        if (fmt[i] == 's') {
            char** str_ptr = (char**)va_arg(ap, char**);
            if (row[col] == NULL)
                *str_ptr = NULL;
            else
                *str_ptr = row[col];
            col++;
        } else if (fmt[i] == 'S')    {
            char** str_ptr = (char**)va_arg(ap, char**);
            if (row[col] == NULL)
                *str_ptr = NULL;
            else
                *str_ptr = strdup(row[col]);
            col++;
        } else if (fmt[i] == 'i')    {
            int* int_ptr = (int*)va_arg(ap, int*);
            if (row[col] != NULL)
                *int_ptr = atoi(row[col]);
            col++;
        } else if (fmt[i] == '?')    { // null flag
            int* int_ptr = (int*)va_arg(ap, int*);
            if (row[col] == NULL)
                *int_ptr = 1;
            else
                *int_ptr = 0;
        } else {
            err = nerr_raise(NERR_ASSERT, "invalid format string %s %c",
                             fmt, fmt[i]);
            va_end(ap);
            break;
        }
    }

    CONN(conn)->row_no++;
    return err;
}

static NEOERR* mysql_mdb_query_geta(mdb_conn* conn, const char* fmt, char* r[])
{
    MYSQL_RES* res = CONN(conn)->res;
    MYSQL_ROW row = CONN(conn)->row;
    int row_no = CONN(conn)->row_no;

    if (res == NULL) return nerr_raise(NERR_DB, "attemp fetch null res");

    if (row_no >= mdb_get_rows(conn)) {
        if (row_no == 0) return nerr_raise(NERR_NOT_FOUND, "empty result");
        return nerr_raise(NERR_OUTOFRANGE, "last row has fetched");
    }

    int param_count = fmt != NULL ? strlen(fmt) : 0;
    int i, col = 0;

    row = mysql_fetch_row(res);
    if (!row) return nerr_raise(NERR_OUTOFRANGE, "last row has fetched");
    if (param_count > mysql_num_fields(res))
        return nerr_raise(NERR_ASSERT, "result col num not match");

    for (i = 0; i < param_count; i++) {
        if (fmt[i] == 's') {
            if (row[col] == NULL)
                r[i] = NULL;
            else
                r[i] = row[col];
        } else if (fmt[i] == 'S')    {
            if (row[col] == NULL)
                r[i] = NULL;
            else
                r[i] = strdup(row[col]);
        }
        col++;
    }

    CONN(conn)->row_no++;
    return STATUS_OK;
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

static NEOERR* mysql_mdb_query_putv(mdb_conn* conn, const char* fmt, va_list ap)
{
    int param_count = (fmt != NULL) ? strlen(fmt) : 0;
    char** param_values = calloc(param_count, sizeof(char*));
    int* free_list = calloc(param_count, sizeof(int));
    int i, col = 0;
    int is_null;
    NEOERR *err = STATUS_OK;

    if (param_values == NULL || free_list == NULL)
        return nerr_raise(NERR_NOMEM, "calloc for query putv failure");

    for (i = 0; i < param_count; i++) {
        is_null = 0;

        if (fmt[i] == '?') {
            is_null = (int)va_arg(ap, int);
            i++;
        }

        if (fmt[i] == 's') {
            char* value = (char*)va_arg(ap, char*);
            if (!is_null)
                if (mysql_add_sql_str(CONN(conn)->mysql,
                                      &(conn->sql), value) != 0) {
                    err = nerr_raise(NERR_ASSERT, "invalid sql string %s %s",
                                     conn->sql, value);
                    goto done;
                }
            col++;
        }
        else if (fmt[i] == 'i')    {
            int value = (int)va_arg(ap, int);
            if (!is_null) {
                if (mysql_add_sql_int(&(conn->sql), value) != 0) {
                    err = nerr_raise(NERR_ASSERT, "invalid sql string %s %d",
                                     conn->sql, value);
                    goto done;
                }
            }
            col++;
        } else {
            err = nerr_raise(NERR_ASSERT, "invalid format string %s %c", fmt, fmt[i]);
            goto done;
        }
    }

    mtc_dbg("%s %d", conn->sql, param_count);
    if (mysql_real_query(CONN(conn)->mysql, conn->sql, strlen(conn->sql)) != 0)
        err = nerr_raise(NERR_DB, "%s %s",
                         conn->sql, mysql_error(CONN(conn)->mysql));

    if (CONN(conn)->res != NULL)
        mysql_free_result(CONN(conn)->res);
    CONN(conn)->res = mysql_store_result(CONN(conn)->mysql);

done:
    for (i = 0; i < param_count; i++)
        if (free_list[i])
            free(param_values[i]);
    free(param_values);
    free(free_list);

    return err;
}

static int mysql_mdb_query_get_rows(mdb_conn* conn)
{
    MYSQL_RES* res = CONN(conn)->res;
    if (res) return (int)mysql_num_rows(res);
    return -1;
}

static int mysql_mdb_query_get_affect_rows(mdb_conn* conn)
{
    return (int)mysql_affected_rows(CONN(conn)->mysql);
}

static int mysql_mdb_query_get_last_id(mdb_conn* conn, const char* seq_name)
{
    return (int)mysql_insert_id(CONN(conn)->mysql);
}

mdb_driver mysql_driver =
{
    .name = "mysql",
    .connect = mysql_mdb_connect,
    .disconnect = mysql_mdb_disconnect,
    .begin = mysql_mdb_begin,
    .commit = mysql_mdb_commit,
    .rollback = mysql_mdb_rollback,
    .query_fill = mysql_mdb_query_fill,
    .query_getv = mysql_mdb_query_getv,
    .query_geta = mysql_mdb_query_geta,
    .query_putv = mysql_mdb_query_putv,
    .query_get_rows = mysql_mdb_query_get_rows,
    .query_get_affect_rows = mysql_mdb_query_get_affect_rows,
    .query_get_last_id = mysql_mdb_query_get_last_id,
};

#else
mdb_driver mysql_driver = {};
#endif    /* DROP_MYSQL */
