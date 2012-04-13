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

NEOERR* mdb_set_row(HDF *hdf, mdb_conn* conn, char *cols, char *prefix, int flags)
{
    if (!conn || !hdf) return nerr_raise(NERR_ASSERT, "param error");

    int qrcnt, i;
    char qrarray[QR_NUM_MAX][LEN_ST];
    char *col[QR_NUM_MAX];
    char fmt[LEN_ST] = {0};
    NEOERR *err;
    
    memset(fmt, 0x0, sizeof(fmt));
    memset(qrarray, 0x0, sizeof(qrarray));
    
    mdb_set_qrarray(cols, qrarray, &qrcnt);
    memset(fmt, 's', qrcnt);

    err = mdb_geta(conn, fmt, col);
    if (flags & MDB_FLAG_EMPTY_OK) nerr_handle(&err, NERR_NOT_FOUND);
    if (err != STATUS_OK) {
        if (flags & MDB_FLAG_NO_ERR) nerr_ignore(&err);
        else return nerr_pass(err);
    }

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

#define BUILD_HDF_FMT()                                                 \
    do {                                                                \
        bool smatch = false;                                            \
        string_clear(&hdfkey);                                          \
        if (prefix) string_appendf(&hdfkey, "%s.", prefix);             \
        if (keycol >= 0) {                                              \
            string_append(&hdfkey, col[keycol]);                        \
        } else {                                                        \
            string_appendf(&hdfkey, "%d", rowsn);                       \
        }                                                               \
        if (cols) {                                                     \
            /* is this col in slaveval[][]? */                          \
            if (slavenum > 0) {                                         \
                int key = 0;                                            \
                for (int si = 0; si < slavenum; si++) {                 \
                    key = slavekey[si];                                 \
                    smatch = false;                                     \
                    for (int sj = 0; sj < QR_NUM_MAX; sj++) {           \
                        if (i == slaveval[si][sj]) {                    \
                            smatch = true;                              \
                            break;                                      \
                        }                                               \
                    }                                                   \
                    if (smatch) {                                       \
                        /* in slaveval[si], append key, and continue */ \
                        string_appendf(&hdfkey, ".%s.%s", qrarray[key], col[key]); \
                    } else {                                            \
                        /* not in, break */                             \
                        break;                                          \
                    }                                                   \
                }                                                       \
            }                                                           \
            string_appendf(&hdfkey, ".%s", qrarray[i]);                 \
        }                                                               \
        string_append(&hdfkey, "=%s");                                  \
    } while (0)

NEOERR* mdb_set_rows(HDF *hdf, mdb_conn* conn, char *cols,
                     char *prefix, char *keyspec, int flags)
{
    if (!conn || !hdf) return nerr_raise(NERR_ASSERT, "param error");

    int qrcnt = 1, i;
    char qrarray[QR_NUM_MAX][LEN_ST];
    char *col[QR_NUM_MAX], fmt[LEN_ST], *ps, *pe;
	STRING hdfkey; string_init(&hdfkey);
    int keycol, slavekey[QR_NUM_MAX], slaveval[QR_NUM_MAX][QR_NUM_MAX], slavenum = 0;
    NEOERR *err;
    
    memset(fmt, 0x0, sizeof(fmt));
    memset(qrarray, 0x0, sizeof(qrarray));
    memset(slavekey, 0x0, sizeof(slavekey));
    memset(slaveval, 0x0, sizeof(slaveval));

    if (cols) mdb_set_qrarray(cols, qrarray, &qrcnt);
    memset(fmt, 's', qrcnt);

    if (keyspec && atoi(keyspec) < qrcnt) {
        /*
         * 0;2:2,3,4,5,6;4:4,5,6
         * TODO space strim: 0; 2:2-6; 4: 4, 5,  6 
         */
        keycol = atoi(keyspec); /* 0 */

        /*
         * decode key spec
         */
        keyspec = strdup(keyspec);
        ps = keyspec;

        while (*ps && *ps++ != ';') {;}
        /*
         * 2:2,3,4,5,6;4:4,5,6
         */

        if (*ps) {
            ULIST *list;
            string_array_split(&list, ps, ";", QR_NUM_MAX);
            /*
             * 2:2,3,4,5,6
             * 4:4,5,6
             */
            
            int k, v, cnt, min, max;
            
            ITERATE_MLIST(list) {
                k = -1; v = -1;
                ps = list->items[t_rsv_i];

                k = atoi(ps);
                while (*ps && *ps++ != ':') {;}
                
                if (k >=0 && *ps) {
                    slavekey[slavenum] = k;

                    /* 2,3,4,5,6 */
                    /* OR 2-4,6 OR 2*/
                    cnt = 0;
                    while (*ps && cnt < QR_NUM_MAX-1) {
                        pe = ps;
                        while (*pe && *pe != ',') {pe++;}

                        if (pe > ps) {
                            if (mstr_isdigitn(ps, pe-ps)) {
                                slaveval[slavenum][cnt++] = atoi(ps);
                            } else if (mstr_israngen(ps, pe-ps, &min, &max)){
                                for (int x = min; x <= max; x++) {
                                    if (cnt >= QR_NUM_MAX-2) break;
                                    slaveval[slavenum][cnt++] = x;
                                }
                            }
                        }

                        while (*pe && *pe == ',') {pe++;};
                        ps = pe;
                    }

                    while (cnt < QR_NUM_MAX) {slaveval[slavenum][cnt++] = -1;}
                    
                    slavenum++;
                }
            }
            
            uListDestroy(&list, ULIST_FREE);
        }
        free(keyspec);
    } else keycol = -1;
    
    /* append to last child */
    int rowsn = 0;
    if (prefix && keycol < 0) {
        HDF *res = hdf_get_child(hdf, prefix);
        while (res != NULL) {
            rowsn++;
            res = hdf_obj_next(res);
        }
    }

    while ( (err = mdb_geta(conn, fmt, col)) == STATUS_OK ){
        for (i = 0; i < qrcnt; i++) {
            BUILD_HDF_FMT();
            hdf_set_valuef(hdf, hdfkey.buf, col[i]);
        }
        rowsn++;
    }

    string_clear(&hdfkey);

    /*
     * last row has fetched
     */
    nerr_handle(&err, NERR_OUTOFRANGE);
    if (flags & MDB_FLAG_EMPTY_OK) nerr_handle(&err, NERR_NOT_FOUND);
    if (err != STATUS_OK) {
        if (flags & MDB_FLAG_NO_ERR) nerr_ignore(&err);
        else return nerr_pass(err);
    }

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

/*
 * util function
 */
void mdb_set_qrarray(char *qrcol, char qr_array[QR_NUM_MAX][LEN_ST], int *qr_cnt)
{
    char src[LEN_ML], tok[LEN_ST];

    int cnt = 0;
    char *p;
    char *b, *e, *bp;
    int pos, level;

    /*
     * prepare src string for strtok. (exactly qrcol string without '(...)')
     * in : Direction, Actor, CONCAT(Sort1,';',Sort2,';',Sort3,';',Sort4,';',Sort5) AS Sort1, ceil(date_part('epoch', intime)*1000) as   intime
     * out: Direction, Actor, CONCAT AS Sort1, ceil as    intime
     */
    memset(src, 0x0, sizeof(src));
    p = qrcol;
    pos = level = 0;
    while (*p && pos < LEN_ML) {
        if (*p != '(' && *p != ')' && level == 0) src[pos++] = *p;
        else {
            if (*p == '(') level++;
            else if (*p == ')') level--;
        }
        p++;
    }
    
    p = strtok(src, ",");
    while (p != NULL) {
        //mtc_noise("parse %dst token: '%s'", cnt, p);
        memset(tok,0,sizeof(tok));
        strncpy(tok, p, sizeof(tok)-1);
        b = tok;
        while(*b && (*b == '\t' || *b == ' ' || *b == '\r' || *b == '\n')) {
            b++;
        }
        e = tok;
        if (strlen(tok) >= 1)
            e += strlen(tok)-1;
        while(*e && (*e == '\t' || *e == ' ' || *e == '\r' || *e == '\n')) {
            e--;
        }
        if (*b == '\0' || *e == '\0') {
            p = strtok(NULL, ",");
            continue;
        }
        strncpy(qr_array[cnt], b, e-b+1);
        //mtc_noise("get tok '%s'", qr_array[cnt]);

        strcpy(tok, qr_array[cnt]);
        bp = strcasestr(tok, " as ");
        if (bp != NULL) {
            //mtc_noise("token '%s' contain ' as '", qr_array[cnt]);
            bp = bp + 4;
            while(*bp && (*bp == '\t' || *bp == ' ' || *bp == '\r' || *bp == '\n')) {
                bp++;
            }
            strncpy(qr_array[cnt], bp, sizeof(qr_array[cnt])-1);
            mtc_info("get tok truely '%s'", qr_array[cnt]);
        }
        
        cnt++;
        p = strtok(NULL, ",");
    }
    *qr_cnt = cnt;
}


NEOERR* mdb_build_upcol(HDF *data, HDF *node, STRING *str)
{
    if (!data || !node || !str) return nerr_raise(NERR_ASSERT, "param err");
    
    char *name, *col, *val, *esc, *require, *clen, *type;

    node = hdf_obj_child(node);
    
    while (node) {
        name = hdf_obj_name(node);
        col = hdf_obj_value(node);
        val = hdf_get_value(data, name, NULL);
        require = mcs_obj_attr(node, "require");
        clen = mcs_obj_attr(node, "maxlen");
        type = mcs_obj_attr(node, "type");
        if (val && *val) {
            if (str->len > 0) {
                string_appendf(str, " , ");
            }
            if (type == NULL || !strcmp(type, "str")) {
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                if (clen)
                    string_appendf(str, " %s='%s'::varchar(%d) ",
                                   col, esc, atoi(clen));
                else
                    string_appendf(str, " %s='%s' ", col, esc);
                free(esc);

            } else if (!strcmp(type, "int")) {
                string_appendf(str, " %s=%d ", col, atoi(val));

            } else if (!strcmp(type, "float")) {
                string_appendf(str, " %s=%f ", col, atof(val));

            } else if (!strcmp(type, "point")) {
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                string_appendf(str, " %s= point '%s' ", col, esc);
                free(esc);

            } else if (!strcmp(type, "box")) {
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                string_appendf(str, " %s= box '%s' ", col, esc);
                free(esc);

            } else if (!strcmp(type, "path")) {
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                string_appendf(str, " %s= path '%s' ", col, esc);
                free(esc);

            } else if (!strcmp(type, "bitop")) {
                char *opkey;
                int opval = 0;
                opkey = mcs_obj_attr(node, "opkey");
                opval = hdf_get_int_value(data, opkey, 0);
                if (opkey) {
                    if (opval == 1) {
                        /* set bit */
                        string_appendf(str, " %s=%s|%u ", col, col,
                                       (unsigned int)atoi(val));
                    } else {
                        /* unset bit */
                        string_appendf(str, " %s=%s&%u ", col, col,
                                       (unsigned int)~atoi(val));
                    }
                } else {
                    return nerr_raise(NERR_ASSERT, "%s don't have opkey", col);
                }
            }
        } else if (require && !strcmp(require, "true")) {
            return nerr_raise(NERR_ASSERT, "require %s %s", name, type);
        }
        
        node = hdf_obj_next(node);
    }

    if (str->len <= 0)
        return nerr_raise(NERR_ASSERT, "str len 0");

    return STATUS_OK;
}

NEOERR* mdb_build_querycond(HDF *data, HDF *node, STRING *str, char *defstr)
{
    if (!data || !node || !str) return nerr_raise(NERR_ASSERT, "param err");
    
    char *name, *col, *val, *esc, *require, *type;

    node = hdf_obj_child(node);
    
    while (node) {
        name = hdf_obj_name(node);
        col = hdf_obj_value(node);
        val = hdf_get_value(data, name, NULL);
        require = mcs_obj_attr(node, "require");
        type = mcs_obj_attr(node, "type");
        if (val && *val) {
            if (str->len > 0) string_append(str, " AND ");
            
            if (type == NULL || !strcmp(type, "str")) {
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                string_appendf(str, " %s '%s' ", col, esc);
                free(esc);
            } else if (!strcmp(type, "raw")) {
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                string_appendf(str, " %s %s ", col, esc);
                free(esc);
            } else if (!strcmp(type, "int")){
                string_appendf(str, " %s %d ", col, atoi(val));

            } else if (!strcmp(type, "float")){
                string_appendf(str, " %s %f ", col, atof(val));

            } else if (!strcmp(type, "point")){
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                string_appendf(str, " %s point '%s' ", col, esc);
                free(esc);

            } else if (!strcmp(type, "box")){
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                string_appendf(str, " %s box '%s' ", col, esc);
                free(esc);

            } else if (!strcmp(type, "path")){
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                string_appendf(str, " %s path '%s' ", col, esc);
                free(esc);
            }
        } else if (require && !strcmp(require, "true")) {
            return nerr_raise(NERR_ASSERT, "require %s %s", name, type);
        }
        
        node = hdf_obj_next(node);
    }
    
    if (str->len <= 0 && defstr) string_append(str, defstr);

    return STATUS_OK;
}

NEOERR* mdb_build_incol(HDF *data, HDF *node, STRING *str)
{
    if (!data || !node || !str) return nerr_raise(NERR_ASSERT, "param err");
    
    char *name, *col, *val, *esc, *require, *clen, *type;
    STRING sa, sb;
    string_init(&sa);
    string_init(&sb);

    node = hdf_obj_child(node);
    
    while (node) {
        name = hdf_obj_name(node);
        col = hdf_obj_value(node);
        val = hdf_get_value(data, name, NULL);
        require = mcs_obj_attr(node, "require");
        clen = mcs_obj_attr(node, "maxlen");
        type = mcs_obj_attr(node, "type");
        if (val && *val) {
            
            if (sa.len <= 0) {
                string_appendf(&sa, " (%s ", col);
                string_appendf(&sb, " VALUES (");
            } else {
                string_appendf(&sa, ", %s ", col);
                string_appendf(&sb, ", ");
            }

            if (type == NULL || !strcmp(type, "str")) {
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                if (clen)
                    string_appendf(&sb, " '%s'::varchar(%d) ",
                                   esc, atoi(clen));
                else
                    string_appendf(&sb, " '%s' ", esc);
                free(esc);

            } else if (!strcmp(type, "int")) {
                string_appendf(&sb, "%d", atoi(val));

            } else if (!strcmp(type, "float")) {
                string_appendf(&sb, "%f", atof(val));

            } else if (!strcmp(type, "point")) {
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                string_appendf(&sb, "point '%s' ", esc);
                free(esc);

            } else if (!strcmp(type, "box")) {
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                string_appendf(&sb, "box '%s' ", esc);
                free(esc);

            } else if (!strcmp(type, "path")) {
                mstr_real_escape_string_nalloc(&esc, val, strlen(val));
                string_appendf(&sb, "path '%s' ", esc);
                free(esc);
            }
        } else if (require && !strcmp(require, "true")) {
            string_clear(&sa);
            string_clear(&sb);
            return nerr_raise(NERR_ASSERT, "require %s %s", name, type);
        }
        
        node = hdf_obj_next(node);
    }

    if (sa.len <= 0) {
        string_clear(&sa);
        string_clear(&sb);
        return nerr_raise(NERR_ASSERT, "str len 0");
    }

    string_appendf(str, "%s)  %s)", sa.buf, sb.buf);

    string_clear(&sa);
    string_clear(&sb);
    
    return STATUS_OK;
}

NEOERR* mdb_build_mgcol(HDF *data, HDF *node, STRING *str)
{
    if (!data || !node || !str) return nerr_raise(NERR_ASSERT, "param err");
    
    char *name, *col, *val, *esc, *require, *clen, *type;

    node = hdf_obj_child(node);
    
    while (node) {
        name = hdf_obj_name(node);
        col = hdf_obj_value(node);
        val = hdf_get_value(data, name, NULL);
        require = mcs_obj_attr(node, "require");
        clen = mcs_obj_attr(node, "maxlen");
        type = mcs_obj_attr(node, "type");

        if (require && !strcmp(require, "true") && (!val || !*val)) {
            return nerr_raise(NERR_ASSERT, "require %s %s", name, type);
        }

        if (!val || !*val) val = "";
        esc = "";

        if (str->len > 0) {
            string_appendf(str, " , ");
        }
        if (type == NULL || !strcmp(type, "str")) {
            mstr_real_escape_string_nalloc(&esc, val, strlen(val));
            if (clen)
                string_appendf(str, " '%s'::varchar(%d) ",
                               esc, atoi(clen));
            else
                string_appendf(str, " '%s' ", esc);
            free(esc);

        } else if (!strcmp(type, "int")) {
            string_appendf(str, "%d", atoi(val));

        } else if (!strcmp(type, "float")) {
            string_appendf(str, "%f", atof(val));

        } else if (!strcmp(type, "point")) {
            if (!val || !*val) val = "(0,0)";
            mstr_real_escape_string_nalloc(&esc, val, strlen(val));
            string_appendf(str, "point '%s' ", esc);
            free(esc);

        } else if (!strcmp(type, "box")) {
            if (!val || !*val) val = "((0,0),(1,1))";
            mstr_real_escape_string_nalloc(&esc, val, strlen(val));
            string_appendf(str, "box '%s' ", esc);
            free(esc);

        } else if (!strcmp(type, "path")) {
            if (!val || !*val) val = "((0,0),(1,1))";
            mstr_real_escape_string_nalloc(&esc, val, strlen(val));
            string_appendf(str, "path '%s' ", esc);
            free(esc);
        } else if (!strcmp(type, "time")) {
            if (!val || !*val) val = "00:00:00";
            mstr_real_escape_string_nalloc(&esc, val, strlen(val));
            string_appendf(str, "time '%s' ", esc);
            free(esc);
        }

        node = hdf_obj_next(node);
    }

    if (str->len <= 0)
        return nerr_raise(NERR_ASSERT, "str len 0");

    return STATUS_OK;
}

void mdb_pagediv(HDF *hdf, char *inprefix, int *count, int *offset,
                 char *outprefix, HDF *ohdf)
{
    char hdfkey[LEN_HDF_KEY];
    int i, j, npg = DFT_PAGE_NUM;

    if (inprefix) snprintf(hdfkey, sizeof(hdfkey), "%s._npp", inprefix);
    else strcpy(hdfkey, "_npp");
    i = hdf_get_int_value(hdf, hdfkey, DFT_NUM_PERPAGE);

    if (outprefix) hdf_set_valuef(ohdf, "%s._npp=%d", outprefix, i);
    else hdf_set_int_value(ohdf, "_npp", i);
    
    if (inprefix) snprintf(hdfkey, sizeof(hdfkey), "%s._nst", inprefix);
    else strcpy(hdfkey, "_nst");
    j = hdf_get_int_value(hdf, hdfkey, -1);
    if (j == -1) {
        if (inprefix) snprintf(hdfkey, sizeof(hdfkey), "%s._npg", inprefix);
        else strcpy(hdfkey, "_npg");
        j = hdf_get_int_value(hdf, hdfkey, DFT_PAGE_NUM);
        npg = j;
        j = (j-1)*i;
    }

    if (outprefix) hdf_set_valuef(ohdf, "%s._npg=%d", outprefix, npg);
    else hdf_set_int_value(ohdf, "_npg", npg);
    
    *count = i;
    *offset = j;

    if (outprefix) hdf_set_valuef(ohdf, "%s._nst=%d", outprefix, j);
    else hdf_set_int_value(ohdf, "_nst", j);
}
