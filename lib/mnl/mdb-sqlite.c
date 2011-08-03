#include "mheads.h"
#ifndef DROP_SQLITE
#include <sqlite3.h>

struct _mdb_conn_sqlite
{
    mdb_conn base;
    sqlite3* handle;
    sqlite3_stmt* stmt;
    int state;
};

enum _sqlite_query_state
{
    QUERY_STATE_INIT,          // just after sqlite3_prepare
    QUERY_STATE_ROW_PENDING,   // sqlite3_step was called and row was not retrieved using getv
    QUERY_STATE_ROW_READ,      // row was retrieved using getv
    QUERY_STATE_COMPLETED,     // no more rows
};

#define CONN(c) ((struct _mdb_conn_sqlite*)(c))

// dsn is filename
static NEOERR* sqlite_mdb_connect(const char* dsn, mdb_conn **rconn)
{
    struct _mdb_conn_sqlite* conn;

    *rconn = NULL;
    conn = calloc(1, sizeof(struct _mdb_conn_sqlite));
    if (sqlite3_open(dsn, &conn->handle) == SQLITE_OK)
        sqlite3_busy_timeout(conn->handle, 30000);
    else
        return nerr_raise(NERR_SYSTEM, "connect to %s failure %s",
                          dsn, sqlite3_errmsg(conn->handle));
    *rconn = (mdb_conn*)conn;
    
    return STATUS_OK;
}

static void sqlite_mdb_disconnect(mdb_conn* conn)
{
    if (sqlite3_close(CONN(conn)->handle) != SQLITE_OK)
        mtc_err("GSQLW: Can't close sqlite: %s\n", sqlite3_errmsg(CONN(conn)->handle));
    if (CONN(conn)->stmt != NULL)    {
        if (sqlite3_finalize(CONN(conn)->stmt) != SQLITE_OK)
            mtc_err("GSQLW: Can't close sqlite query: %s\n", sqlite3_errmsg(CONN(conn)->handle));
        else
            CONN(conn)->stmt = NULL;
    }
}

static NEOERR* sqlite_mdb_begin(mdb_conn* conn)
{
    return mdb_exec(conn, NULL, "BEGIN TRANSACTION", NULL);
}

static NEOERR* sqlite_mdb_commit(mdb_conn* conn)
{
    return mdb_exec(conn, NULL, "COMMIT TRANSACTION", NULL);
}

static NEOERR* sqlite_mdb_rollback(mdb_conn* conn)
{
    return mdb_exec(conn, NULL, "ROLLBACK TRANSACTION", NULL);
}

static char* _sqlite_fixup_sql(const char* str)
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

static NEOERR* sqlite_mdb_query_fill(mdb_conn* conn, const char* sql_string)
{
    if (conn->sql) free(conn->sql);
    conn->sql = NULL;
    conn->sql = _sqlite_fixup_sql(sql_string);

    CONN(conn)->state = QUERY_STATE_INIT;
    if (CONN(conn)->stmt != NULL)    {
        if (sqlite3_finalize(CONN(conn)->stmt) != SQLITE_OK)
            return nerr_raise(NERR_DB, "GSQLW: Can't close sqlite query: %s\n",
                              sqlite3_errmsg(CONN(conn)->handle));
    }

    if (conn->sql != NULL) {
        if (sqlite3_prepare_v2(CONN(conn)->handle, conn->sql, -1,
                               &CONN(conn)->stmt, NULL) != SQLITE_OK)
            return nerr_raise(NERR_DB, "prepare sql %s failure %s",
                              sql_string, sqlite3_errmsg(CONN(conn)->handle));
    }
    
    return STATUS_OK;
}

static NEOERR* sqlite_mdb_query_getv(mdb_conn* conn, const char* fmt, va_list ap)
{
    sqlite3_stmt* stmt = CONN(conn)->stmt;
    int param_count = fmt != NULL ? strlen(fmt) : 0;
    int i, rs;
    int col = 0;
    NEOERR *err = STATUS_OK;

    switch (CONN(conn)->state) {
    case QUERY_STATE_INIT:
        return nerr_raise(NERR_ASSERT, "call mdb_put() before mdb_get() pls");
    case QUERY_STATE_COMPLETED:
        return nerr_raise(NERR_OUTOFRANGE, "last row has fetched");
    case QUERY_STATE_ROW_READ:
        // fetch next row
        rs = sqlite3_step(stmt);
        if (rs == SQLITE_ROW) break;
        else if (rs == SQLITE_DONE) {
            CONN(conn)->state = QUERY_STATE_COMPLETED;
            return nerr_raise(NERR_OUTOFRANGE, "last row has fetched");
        } else {
            return nerr_raise(NERR_DB, "get %s failure %s",
                              fmt, sqlite3_errmsg(CONN(conn)->handle));
        }
    case QUERY_STATE_ROW_PENDING:
        CONN(conn)->state = QUERY_STATE_ROW_READ;
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
            err = nerr_raise(NERR_ASSERT, "invalid format string %s %c",
                             fmt, fmt[i]);
            va_end(ap);
            break;
        }
    }

    return err;
}

static NEOERR* sqlite_mdb_query_geta(mdb_conn* conn, const char* fmt, char *r[])
{
    sqlite3_stmt* stmt = CONN(conn)->stmt;
    int param_count = fmt != NULL ? strlen(fmt) : 0;
    int i, rs;
    int col = 0;

    switch (CONN(conn)->state) {
    case QUERY_STATE_INIT: 
        return nerr_raise(NERR_ASSERT, "call mdb_put() before mdb_get() pls");
    case QUERY_STATE_COMPLETED:
        return nerr_raise(NERR_OUTOFRANGE, "last row has fetched");
    case QUERY_STATE_ROW_READ:
        // fetch next row
        rs = sqlite3_step(stmt);
        if (rs == SQLITE_ROW) break;
        else if (rs == SQLITE_DONE) {
            CONN(conn)->state = QUERY_STATE_COMPLETED;
            return nerr_raise(NERR_OUTOFRANGE, "last row has fetched");
        } else {
            return nerr_raise(NERR_DB, "get %s failure %s",
                              fmt, sqlite3_errmsg(CONN(conn)->handle));
        }
    case QUERY_STATE_ROW_PENDING:
        CONN(conn)->state = QUERY_STATE_ROW_READ;
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

    return STATUS_OK;
}

static NEOERR* sqlite_mdb_query_putv(mdb_conn* conn, const char* fmt, va_list ap)
{
    sqlite3_stmt* stmt = CONN(conn)->stmt;
    int param_count = (fmt != NULL) ? strlen(fmt) : 0;
    int i, rs;
    int col = 1;

    if (CONN(conn)->state != QUERY_STATE_INIT) {
        if (sqlite3_reset(stmt) != SQLITE_OK)
            return nerr_raise(NERR_DB, "reset sqlite db failure %s",
                              sqlite3_errmsg(CONN(conn)->handle));
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
        else if (fmt[i] == 'i')    {
            int value = (int)va_arg(ap, int);
            if (!is_null)
                sqlite3_bind_int(stmt, col, value);
            col++;
        } else {
            return nerr_raise(NERR_ASSERT, "invalid format string %s %c",
                              fmt, fmt[i]);
        }
    }

    mtc_dbg("%s %s %d", conn->sql, fmt, param_count);
    rs = sqlite3_step(stmt);
    if (rs == SQLITE_DONE)
        CONN(conn)->state = QUERY_STATE_COMPLETED;
    else if (rs == SQLITE_ROW)
        CONN(conn)->state = QUERY_STATE_ROW_PENDING;
    else
        return nerr_raise(NERR_DB, "step db %s %s %d failure %s",
                          conn->sql, fmt, param_count, sqlite3_errmsg(CONN(conn)->handle));

    return STATUS_OK;
}

static int sqlite_mdb_query_get_rows(mdb_conn* conn)
{
    int rs;
    int count = 0;
    sqlite3_stmt* stmt = CONN(conn)->stmt;

    if (CONN(conn)->state == QUERY_STATE_INIT) {
        mtc_err("call mdb_query_put() before mdb_query_get_rows().");
        return -1;
    }

    if (sqlite3_reset(stmt) != SQLITE_OK) {
        mtc_err("reset db failure %s", sqlite3_errmsg(CONN(conn)->handle));
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
            mtc_err("step db failure %s", sqlite3_errmsg(CONN(conn)->handle));
            return -1;
        }
    }

    if (sqlite3_reset(stmt) != SQLITE_OK) {
        mtc_err("reset db failure %s", sqlite3_errmsg(CONN(conn)->handle));
        return -1;
    }

    rs = sqlite3_step(stmt);
    if (rs == SQLITE_ROW)
        CONN(conn)->state = QUERY_STATE_ROW_PENDING;
    else if (rs == SQLITE_DONE)
        CONN(conn)->state = QUERY_STATE_COMPLETED;
    else {
        //XXX: set error based on sqlite state
        mtc_err("step db failure %s", sqlite3_errmsg(CONN(conn)->handle));
        return -1;
    }

    return count;
}

static int sqlite_mdb_query_get_affect_rows(mdb_conn* conn)
{
    mtc_err("sqlite_mdb_query_get_affect_rows() is not implemented!");
    return -1;
}

static int sqlite_mdb_query_get_last_id(mdb_conn* conn, const char* seq_name)
{
    return sqlite3_last_insert_rowid(CONN(conn)->handle);
}

mdb_driver sqlite_driver =
{
    .name = "sqlite",
    .connect = sqlite_mdb_connect,
    .disconnect = sqlite_mdb_disconnect,
    .begin = sqlite_mdb_begin,
    .commit = sqlite_mdb_commit,
    .rollback = sqlite_mdb_rollback,
    .query_fill = sqlite_mdb_query_fill,
    .query_getv = sqlite_mdb_query_getv,
    .query_geta = sqlite_mdb_query_geta,
    .query_putv = sqlite_mdb_query_putv,
    .query_get_rows = sqlite_mdb_query_get_rows,
    .query_get_affect_rows = sqlite_mdb_query_get_affect_rows,
    .query_get_last_id = sqlite_mdb_query_get_last_id,
};

#else
mdb_driver sqlite_driver = {};
#endif    /* DROP_SQLITE */
