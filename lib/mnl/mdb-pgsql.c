#include "mheads.h"
#ifndef DROP_PG
#include <libpq-fe.h>

struct _mdb_conn_pgsql
{
    mdb_conn base;
    PGconn* pg;
    PGresult* pg_res;
    int row_no;
};

#define CONN(c) ((struct _mdb_conn_pgsql*)(c))

void notices_black_hole(void* arg, const char* message)
{
    ;
}

static NEOERR* pgsql_mdb_connect(const char* dsn, mdb_conn **rconn)
{
    struct _mdb_conn_pgsql* conn;
  
    *rconn = NULL;
    conn = calloc(1, sizeof(struct _mdb_conn_pgsql));
    if (!conn) return nerr_raise(NERR_NOMEM, "calloc for ms connect failure");
    *rconn = (mdb_conn*)conn;

    conn->pg = PQconnectdb(dsn);
    if (PQstatus(conn->pg) == CONNECTION_BAD)
        return nerr_raise(NERR_SYSTEM, "connect to %s failure %s",
                          dsn, PQerrorMessage(conn->pg));
    PQsetNoticeProcessor(conn->pg, notices_black_hole, NULL);

    return STATUS_OK;
}

static void pgsql_mdb_disconnect(mdb_conn* conn)
{
    if (CONN(conn)->pg)
        PQfinish(CONN(conn)->pg);
    if (CONN(conn)->pg_res != NULL)
        PQclear(CONN(conn)->pg_res);
    CONN(conn)->pg_res = NULL;
}

static NEOERR* pgsql_mdb_begin(mdb_conn* conn)
{
    PGresult* res;
    NEOERR *err = STATUS_OK;
  
    res = PQexec(CONN(conn)->pg, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        err = nerr_raise(NERR_DB, PQresultErrorMessage(res));
    PQclear(res);
    
    return err;
}

static NEOERR* pgsql_mdb_commit(mdb_conn* conn)
{
    PGresult* res;
    NEOERR *err = STATUS_OK;
  
    res = PQexec(CONN(conn)->pg, "COMMIT");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        err = nerr_raise(NERR_DB, PQresultErrorMessage(res));
    PQclear(res);
    
    return err;
}

static NEOERR* pgsql_mdb_rollback(mdb_conn* conn)
{
    PGresult* res;
    NEOERR *err = STATUS_OK;
  
    res = PQexec(CONN(conn)->pg, "ROLLBACK");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        err = nerr_raise(NERR_DB, PQresultErrorMessage(res));
    PQclear(res);
    
    return err;
}

static NEOERR* pgsql_mdb_query_fill(mdb_conn* conn, const char* sql_string)
{
    if (CONN(conn)->pg_res != NULL)
        PQclear(CONN(conn)->pg_res);
    CONN(conn)->pg_res = NULL;
    CONN(conn)->row_no = 0;
    
    if (conn->sql) free(conn->sql);
    conn->sql = NULL;
    if (sql_string)    conn->sql = strdup(sql_string);

    return STATUS_OK;
}

static NEOERR* pgsql_mdb_query_getv(mdb_conn* conn, const char* fmt, va_list ap)
{
    PGresult* res = CONN(conn)->pg_res;
    int row_no = CONN(conn)->row_no;
    NEOERR *err = STATUS_OK;

    if (res == NULL) return nerr_raise(NERR_DB, "attemp fetch null res");

    if (row_no >= mdb_get_rows(conn)) {
        if (row_no == 0) return nerr_raise(NERR_NOT_FOUND, "empty result");
        return nerr_raise(NERR_OUTOFRANGE, "last row has fetched");
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
        } else if (fmt[i] == 'S')    {
            char** str_ptr = (char**)va_arg(ap, char**);
            if (PQgetisnull(res, row_no, col))
                *str_ptr = NULL;
            else
                *str_ptr = strdup(PQgetvalue(res, row_no, col));
            col++;
        } else if (fmt[i] == 'i')    {
            int* int_ptr = (int*)va_arg(ap, int*);
            if (!PQgetisnull(res, row_no, col))
                *int_ptr = atoi(PQgetvalue(res, row_no, col));
            col++;
        } else if (fmt[i] == '?')    { // null flag
            int* int_ptr = (int*)va_arg(ap, int*);
            *int_ptr = PQgetisnull(res, row_no, col);
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

static NEOERR* pgsql_mdb_query_geta(mdb_conn* conn, const char* fmt, char* r[])
{
    PGresult* res = CONN(conn)->pg_res;
    int row_no = CONN(conn)->row_no;

    if (res == NULL) return nerr_raise(NERR_DB, "attemp fetch null res");

    if (row_no >= mdb_get_rows(conn)) {
        if (row_no == 0) return nerr_raise(NERR_NOT_FOUND, "empty result");
        return nerr_raise(NERR_OUTOFRANGE, "last row has fetched");
    }

    int param_count = fmt != NULL ? strlen(fmt) : 0;
    int i, col = 0;

    for (i = 0; i < param_count; i++) {
        if (fmt[i] == 's') {
            if (PQgetisnull(res, row_no, col))
                r[i] = NULL;
            else
                r[i] = PQgetvalue(res, row_no, col);
        } else if (fmt[i] == 'S')    {
            if (PQgetisnull(res, row_no, col))
                r[i] = NULL;
            else
                r[i] = strdup(PQgetvalue(res, row_no, col));
        }
        col++;
    }

    CONN(conn)->row_no++;

    return STATUS_OK;
}

static NEOERR* pgsql_mdb_query_putv(mdb_conn* conn, const char* fmt, va_list ap)
{
    int param_count = (fmt != NULL) ? strlen(fmt) : 0;
    char** param_values = calloc(param_count, sizeof(char*));
    int* free_list = calloc(param_count, sizeof(int));
    int i, col = 0;
    int is_null;
    PGresult* res;
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
                param_values[col] = value;
            col++;
        }
        else if (fmt[i] == 'i')    {
            int value = (int)va_arg(ap, int);
            char cvalue[64];
            if (!is_null) {
                sprintf(cvalue, "%d", value);
                param_values[col] = strdup(cvalue);
                free_list[col] = 1;
            }
            col++;
        } else {
            err = nerr_raise(NERR_ASSERT, "invalid format string %s %c",
                             fmt, fmt[i]);
            goto done;
        }
    }
    param_count = col;

    mtc_dbg("%s %s %d", conn->sql, fmt, param_count);
    for (int i = 0; i < param_count; i++) {
        mtc_dbg("%s", param_values[i]);
    }
    res = PQexecParams(CONN(conn)->pg, conn->sql, param_count, NULL,
                       (const char* const*)param_values, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK &&
        PQresultStatus(res) != PGRES_TUPLES_OK)
        err = nerr_raise(NERR_DB, "%s %s", conn->sql, PQresultErrorMessage(res));

    if (CONN(conn)->pg_res != NULL)
        PQclear(CONN(conn)->pg_res);
    CONN(conn)->pg_res = res;
  
done:
    for (i = 0; i < param_count; i++)
        if (free_list[i])
            free(param_values[i]);
    free(param_values);
    free(free_list);

    return err;
}

static int pgsql_mdb_query_get_rows(mdb_conn* conn)
{
    PGresult* res = CONN(conn)->pg_res;
    if (res) return PQntuples(res);
    return -1;
}

static int pgsql_mdb_query_get_affect_rows(mdb_conn* conn)
{
    PGresult* res = CONN(conn)->pg_res;
    if (res) return atoi(PQcmdTuples(res));
    return -1;
}

static int pgsql_mdb_query_get_last_id(mdb_conn* conn, const char* seq_name)
{
    mtc_err("pgsql_mdb_query_get_last_id() is not implemented!");
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
    .query_fill = pgsql_mdb_query_fill,
    .query_getv = pgsql_mdb_query_getv,
    .query_geta = pgsql_mdb_query_geta,
    .query_putv = pgsql_mdb_query_putv,
    .query_get_rows = pgsql_mdb_query_get_rows,
    .query_get_affect_rows = pgsql_mdb_query_get_affect_rows,
    .query_get_last_id = pgsql_mdb_query_get_last_id,
};

#else
mdb_driver pgsql_driver = {};
#endif    /* DROP_PG */
