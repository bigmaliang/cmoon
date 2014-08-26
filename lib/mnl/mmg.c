#include "mheads.h"

#define GET_LAST_ERROR(con, dbname)                                 \
    do {                                                            \
        m_errmsg = (char*) mongo_sync_conn_get_last_error(con);     \
    } while (0)

static char *m_errmsg = NULL;

NEOERR* mmg_init(char *host, int port, int ms, mmg_conn **db)
{
    if (!host) return nerr_raise(NERR_ASSERT, "paramter null");

    mmg_conn *ldb;

    mtc_noise("connect to %s %d %d ...", host, port, ms);

    *db = NULL;
    ldb = calloc(1, sizeof(mmg_conn));
    if (!ldb) return nerr_raise(NERR_NOMEM, "calloc for connection");

    ldb->con = mongo_sync_connect(host, port, true);
    if (!ldb->con) {
        GET_LAST_ERROR(ldb->con, NULL);
        return nerr_raise(NERR_DB, "sync connect: %s %d %s",
                          m_errmsg, errno, strerror(errno));
    }
    mongo_sync_conn_set_auto_reconnect(ldb->con, true);
    mongo_sync_conn_set_safe_mode(ldb->con, true);
    if (ms > 0) mongo_connection_set_timeout((mongo_connection*)ldb->con, ms);

    *db = ldb;

    return STATUS_OK;
}

NEOERR* mmg_auth(mmg_conn *db, char *dsn, char *user, char *pass)
{
    mtc_noise("authorize to %s use %s %s", dsn, user, pass);

    if (!mongo_sync_cmd_authenticate(db->con, dsn, user, pass)) {
        GET_LAST_ERROR(db->con, dsn);
        return nerr_raise(NERR_DB, "auth: %s %d %s", m_errmsg, errno, strerror(errno));
    }

    return STATUS_OK;
}

NEOERR* mmg_seed_add(mmg_conn *db, char *host, int port)
{
    mtc_noise("add seed %s %d ...", host, port);

    if (!mongo_sync_conn_seed_add(db->con, host, port)) {
        GET_LAST_ERROR(db->con, NULL);
        return nerr_raise(NERR_DB, "add seed: %s %d %s",
                          m_errmsg, errno, strerror(errno));
    }

    return STATUS_OK;
}

void mmg_destroy(mmg_conn *db)
{
    if (!db) return;

    if (db->c) {
        mongo_sync_cursor_free(db->c);
        db->c = NULL;
    }
    /* TODO packet */
    if (db->docs) {
        bson_free(db->docs);
        db->docs = NULL;
    }
    if (db->docq) {
        bson_free(db->docq);
        db->docq = NULL;
    }

    mongo_sync_disconnect(db->con);
}

NEOERR* mmg_prepare(mmg_conn *db, int flags, int skip, int limit,
                    NEOERR* (*qcbk)(mmg_conn *db, HDF *node, bool lastone),
                    char *sels, char *querys)
{
    if (!db || !querys) return nerr_raise(NERR_ASSERT, "paramter null");

    mtc_noise("prepare %s %s", querys, sels);

    /*
     * doc query
     */
    if (db->docq) {
        bson_free(db->docq);
        db->docq = NULL;
    }
    db->docq = mbson_new_from_string(querys, true);
    if (!db->docq) return nerr_raise(NERR_ASSERT, "build query: %s: %s",
                                     querys, strerror(errno));


    /*
     * doc selector
     */
    if (db->docs) {
        bson_free(db->docs);
        db->docs = NULL;
    }
    if (sels) {
        db->docs = mbson_new_from_string(sels, false);
        if (!db->docs) return nerr_raise(NERR_ASSERT, "build selector: %s: %s",
                                         sels, strerror(errno));
        if (!(flags & MMG_FLAG_GETOID)) bson_append_int32(db->docs, "_id", 0);
        bson_finish(db->docs);
    } else if (!(flags & MMG_FLAG_GETOID)) {
        db->docs = bson_build(BSON_TYPE_INT32, "_id", 0, BSON_TYPE_NONE);
        bson_finish(db->docs);
    }

    /*
     * later mmg_prepare won't overwrite formal's callback
     */
    if (db->incallback && qcbk != NULL)
        return nerr_raise(NERR_ASSERT, "already in callback, can't set callback");
    if (!db->incallback) db->query_callback = qcbk;
    db->flags = flags;
    db->skip  = skip;
    db->limit = limit;
    /* callback won't overwrite caller's rescount */
    if (!db->incallback) db->rescount = 0;

    return STATUS_OK;
}

NEOERR* mmg_preparef(mmg_conn *db, int flags, int skip, int limit,
                     NEOERR* (*qcbk)(mmg_conn *db, HDF *node, bool lastone),
                     char *sels, char *qfmt, ...)
{
    char *querys;
    va_list ap;
    NEOERR *err;

    va_start(ap, qfmt);
    querys = vsprintf_alloc(qfmt, ap);
    va_end(ap);
    if (!querys) return nerr_raise(NERR_NOMEM, "Unable to allocate mem for query string");

    err = mmg_prepare(db, flags, skip, limit, qcbk, sels, querys);
    if (err != STATUS_OK) return nerr_pass(err);

    free(querys);
    return STATUS_OK;
}

void mmg_set_callbackdata(mmg_conn *db, void *data)
{
    if (!db || !data) return;

    db->callbackdata = data;
}

NEOERR* mmg_query(mmg_conn *db, char *dsn, char *prefix, HDF *outnode)
{
    int count, thiscount;
    char key[LEN_HDF_KEY];
    HDF *node, *cnode;
    bson *doc;
    NEOERR *err;

    MCS_NOT_NULLB(db, dsn);

    db->p = mongo_sync_cmd_query(db->con, dsn, db->flags & 0x3FF, db->skip, db->limit,
                                 db->docq, db->docs);
    if (!db->p) {
        mtc_noise("queried %s 0 result", dsn);
        if (errno == ENOENT) {
            if (db->flags & MMG_FLAG_EMPTY) {
                if (db->query_callback && !db->incallback) {
                    /*
                     * empty result, call callback
                     */
                    db->incallback = true;

                    err = db->query_callback(db, NULL, true);
                    TRACE_NOK(err);

                    db->incallback = false;

                    db->query_callback = NULL;
                    db->callbackdata = NULL;
                }
                return STATUS_OK;
            }
            return nerr_raise(NERR_NOT_FOUND, "%s 无此记录", dsn);
        }
        GET_LAST_ERROR(db->con, dsn);
        return nerr_raise(NERR_DB, "query: %s %d %s",
                          m_errmsg, errno, strerror(errno));
    }

    /*
     * process result
     */
    if (outnode || (db->query_callback && !db->incallback)) {
        if (outnode) node = outnode; /* need store result */
        else hdf_init(&node);

        db->c = mongo_sync_cursor_new(db->con, dsn, db->p);
        if (!db->c) return nerr_raise(NERR_DB, "cursor: %s", strerror(errno));

        cnode = NULL;
        count = mcs_get_child_num(node, prefix);
        thiscount = 0;
        while (mongo_sync_cursor_next(db->c) && thiscount < db->limit) {
            memset(key, sizeof(key), 0x0);

            if (prefix) {
                if (strchr(prefix, '$')) {
                    HDF *tnode;
                    hdf_init(&tnode);
                    doc = mongo_sync_cursor_get_data(db->c);
                    err = mbson_export_to_hdf(tnode, doc, NULL, MBSON_EXPORT_TYPE, true);
                    if (err != STATUS_OK) return nerr_pass(err);

                    char *tkey = mcs_repvstr_byhdf(prefix, '$', tnode);

                    if (!(db->flags & MMG_FLAG_MIXROWS) && db->limit > 1)
                        snprintf(key, sizeof(key), "%s.%d", tkey, count);
                    else snprintf(key, sizeof(key), "%s", tkey);

                    hdf_copy(node, key, tnode);

                    /*
                     * cnode point to:
                     *   1st $node on strchr(key, '$')
                     * e.g.
                     *   15111231681.name           ===> 15111231681
                     *   levels.2.info              ===> levels.2
                     *   prog.info.levels.2.class.7 ===> prog.info.levels.2
                     */
                    if (!cnode) {
                        char *lprefix = strdup(prefix);
                        char *p = lprefix;
                        if (*p == '$') cnode = node;
                        else {
                            while (*p) {
                                if (*(p+1) == '$') {
                                    *p = '\0';
                                    break;
                                }
                                p++;
                            }
                            cnode = hdf_get_child(node, lprefix);
                        }
                        SAFE_FREE(lprefix);
                    }

                    hdf_destroy(&tnode);
                    SAFE_FREE(tkey);

                    goto nextcursor;
                } else {
                    if (!(db->flags & MMG_FLAG_MIXROWS) && db->limit > 1)
                        snprintf(key, sizeof(key), "%s.%d", prefix, count);
                    else snprintf(key, sizeof(key), "%s", prefix);
                }
            } else {
                if (!(db->flags & MMG_FLAG_MIXROWS) && db->limit > 1)
                    sprintf(key, "%d", count);
                else key[0] = '\0';
            }

            doc = mongo_sync_cursor_get_data(db->c);
            err = mbson_export_to_hdf(node, doc, key, MBSON_EXPORT_TYPE, true);
            if (err != STATUS_OK) return nerr_pass(err);

            /*
             * cnode point to:
             *   key node on !strchr(key, '$')
             * e.g.
             *   user                       ===> user
             *   user.7                     ===> user.7
             */
            if (!cnode) cnode = hdf_get_obj(node, key);

        nextcursor:
            count++;
            thiscount++;
            /* callback won't overwrite caller's rescount */
            if (!db->incallback) db->rescount++;
        }

        mongo_sync_cursor_free(db->c);
        db->c = NULL;
        db->p = NULL;

        mtc_noise("queried %s %d result", dsn, count);

        /*
         * call callback at last. because we don't want declare more mmg_conn*
         * it's safe to do new query in callback on result stored (db->c freeed)
         * we can call mmg_query() recursive, the callback won't.
         */
        if (db->query_callback && !db->incallback) {
            db->incallback = true;

            count = 0;
            while (cnode && count < thiscount) {
                count++;
                if (count == thiscount) err = db->query_callback(db, cnode, true);
                else err = db->query_callback(db, cnode, false);
                TRACE_NOK(err);

                cnode = hdf_obj_next(cnode);
            }

            db->incallback = false;

            /*
             * query_callback can't be shared with multiply query
             * later query must set them again even if TheSameOne
             */
            db->query_callback = NULL;
            db->callbackdata = NULL;
        }

        if (!outnode) hdf_destroy(&node);
    } else {
        /* don't need result */
        mongo_wire_packet_free(db->p);
        db->c = NULL;
        db->p = NULL;
    }

    return STATUS_OK;
}

NEOERR* mmg_string_insert(mmg_conn *db, char *dsn, char *str)
{
    bson *doc;

    MCS_NOT_NULLC(db, dsn, str);

    mtc_noise("insert string %s %s", dsn, str);

    doc = mbson_new_from_string(str, true);
    if (!doc) return nerr_raise(NERR_ASSERT, "build doc: %s: %s",
                                str, strerror(errno));

    if (!mongo_sync_cmd_insert(db->con, dsn, doc, NULL)) {
        GET_LAST_ERROR(db->con, dsn);
        return nerr_raise(NERR_DB, "sync_cmd_insert: %s %d %s",
                          m_errmsg, errno, strerror(errno));
    }

    bson_free(doc);

    return STATUS_OK;
}

NEOERR* mmg_string_insertf(mmg_conn *db, char *dsn, char *fmt, ...)
{
    char *qa;
    va_list ap;
    NEOERR *err;

    va_start(ap, fmt);
    qa = vsprintf_alloc(fmt, ap);
    va_end(ap);
    if (!qa) return nerr_raise(NERR_NOMEM, "Unable to allocate mem for string");

    err = mmg_string_insert(db, dsn, qa);
    if (err != STATUS_OK) return nerr_pass(err);

    free(qa);

    return STATUS_OK;
}

NEOERR* mmg_hdf_insert(mmg_conn *db, char *dsn, HDF *node)
{
    bson *doc;
    NEOERR *err;

    MCS_NOT_NULLC(db, dsn, node);

    char *ts;
    err = hdf_write_string(node, &ts);
    if (err != STATUS_OK) return nerr_pass(err);
    mtc_noise("insert hdf %s %s", dsn, ts);
    SAFE_FREE(ts);

    err = mbson_import_from_hdf(node, &doc, true);
    if (err != STATUS_OK) return nerr_pass(err);

    if (!mongo_sync_cmd_insert(db->con, dsn, doc, NULL)) {
        GET_LAST_ERROR(db->con, dsn);
        return nerr_raise(NERR_DB, "sync_cmd_insert: %s %d %s",
                          m_errmsg, errno, strerror(errno));
    }

    bson_free(doc);

    return STATUS_OK;
}

NEOERR* mmg_hdf_insertl(mmg_conn *db, char *dsn, HDF *node, HDF *lnode)
{
    HDF *inode, *cnode;
    NEOERR *err;

    MCS_NOT_NULLB(db, dsn);
    MCS_NOT_NULLB(node, lnode);

    hdf_init(&inode);

    cnode = hdf_obj_child(lnode);
    while (cnode) {
        char *name = hdf_obj_name(cnode);
        char *key = hdf_obj_value(cnode);
        char *val = hdf_get_value(node, key, NULL);
        char *require = mcs_obj_attr(cnode, "require");
        char *dft = mcs_obj_attr(cnode, "default");
        int type = mcs_get_int_attr(cnode, NULL, "type", CNODE_TYPE_STRING);

        mcs_set_int_attrr(inode, name, "type", type);

        if (type == CNODE_TYPE_TIMESTAMP && !strcmp(key, "_NOW")) {
            mcs_set_int64_value(inode, name, time(NULL));
        } else if (val && *val) {
            hdf_set_value(inode, name, val);
        } else if (require && !strcmp(require, "true")) {
            err = nerr_raise(NERR_ASSERT, "需要 %s %d 参数", name, type);
            goto done;
        } else {
            /*
             * set default value for inserted record
             */
            if (!dft) dft = "";
            hdf_set_value(inode, name, dft);
        }

        cnode = hdf_obj_next(cnode);
    }

    err = mmg_hdf_insert(db, dsn, inode);

done:
    hdf_destroy(&inode);

    return nerr_pass(err);
}

NEOERR* mmg_string_update(mmg_conn *db, char *dsn, int flags, char *up, char *sel)
{
    bson *doca, *docb;

    MCS_NOT_NULLB(db, dsn);
    MCS_NOT_NULLB(up, sel);

    mtc_noise("update %s %s %s", dsn, up, sel);

    doca = mbson_new_from_string(sel, true);
    if (!doca) return nerr_raise(NERR_ASSERT, "build doc sel: %s: %s",
                                 sel, strerror(errno));

    docb = mbson_new_from_string(up, true);
    if (!docb) return nerr_raise(NERR_ASSERT, "build doc up: %s: %s",
                                 up, strerror(errno));

    if (!mongo_sync_cmd_update(db->con, dsn, flags, doca, docb)) {
        GET_LAST_ERROR(db->con, dsn);
        return nerr_raise(NERR_DB, "sync_cmd_update: %s %d %s",
                          m_errmsg, errno, strerror(errno));
    }

    bson_free(doca);
    bson_free(docb);

    return STATUS_OK;
}

NEOERR* mmg_string_updatef(mmg_conn *db, char *dsn, int flags, char *up, char *selfmt, ...)
{
    char *qa = NULL;
    va_list ap;
    NEOERR *err;

    if (selfmt) {
        va_start(ap, selfmt);
        qa = vsprintf_alloc(selfmt, ap);
        va_end(ap);
        if (!qa) return nerr_raise(NERR_NOMEM, "Unable to allocate mem for string");
    }

    err = mmg_string_update(db, dsn, flags, up, qa);

    SAFE_FREE(qa);

    return nerr_pass(err);
}

NEOERR* mmg_hdf_update(mmg_conn *db, char *dsn, int flags, HDF *node, char *sel)
{
    bson *doca, *docb;
    HDF *tnode;
    NEOERR *err;

    MCS_NOT_NULLB(db, dsn);
    MCS_NOT_NULLB(node, sel);

    tnode = node;
    if (flags && MMG_FLAG_UPSET) {
        hdf_init(&tnode);
        hdf_copy(tnode, "$set", node);
    }

    char *ts;
    err = hdf_write_string(tnode, &ts);
    if (err != STATUS_OK) return nerr_pass(err);
    mtc_noise("update %s %s %s", dsn, ts, sel);
    SAFE_FREE(ts);

    doca = mbson_new_from_string(sel, true);
    if (!doca) return nerr_raise(NERR_ASSERT, "build doc sel: %s: %s",
                                 sel, strerror(errno));

    err = mbson_import_from_hdf(tnode, &docb, true);
    if (err != STATUS_OK) return nerr_pass(err);

    if (!mongo_sync_cmd_update(db->con, dsn, flags, doca, docb)) {
        GET_LAST_ERROR(db->con, dsn);
        return nerr_raise(NERR_DB, "sync_cmd_update: %s %d %s",
                          m_errmsg, errno, strerror(errno));
    }

    bson_free(doca);
    bson_free(docb);

    if (flags && MMG_FLAG_UPSET) hdf_destroy(&tnode);

    return STATUS_OK;
}

NEOERR* mmg_hdf_updatef(mmg_conn *db, char *dsn, int flags, HDF *node, char *selfmt, ...)
{
    char *qa = NULL;
    va_list ap;
    NEOERR *err;

    if (selfmt) {
        va_start(ap, selfmt);
        qa = vsprintf_alloc(selfmt, ap);
        va_end(ap);
        if (!qa) return nerr_raise(NERR_NOMEM, "Unable to allocate mem for string");
    }

    err = mmg_hdf_update(db, dsn, flags, node, qa);

    SAFE_FREE(qa);

    return nerr_pass(err);
}

NEOERR* mmg_hdf_updatefl(mmg_conn *db, char *dsn, int flags, HDF *node, HDF *lnode,
                         char *selfmt, ...)
{
    char *qa = NULL;
    HDF *unode, *cnode;
    va_list ap;
    NEOERR *err;

    MCS_NOT_NULLB(db, dsn);
    MCS_NOT_NULLB(node, lnode);

    hdf_init(&unode);

    if (selfmt) {
        va_start(ap, selfmt);
        qa = vsprintf_alloc(selfmt, ap);
        va_end(ap);
        if (!qa) return nerr_raise(NERR_NOMEM, "Unable to allocate mem for string");
    }

    cnode = hdf_obj_child(lnode);
    while (cnode) {
        char *name = hdf_obj_name(cnode);
        char *key = hdf_obj_value(cnode);
        char *val = hdf_get_value(node, key, NULL);
        char *require = mcs_obj_attr(cnode, "require");
        int type = mcs_get_int_attr(cnode, NULL, "type", CNODE_TYPE_STRING);

        if (type == CNODE_TYPE_TIMESTAMP && !strcmp(key, "_NOW")) {
            mcs_set_int64_value(unode, name, time(NULL));
            mcs_set_int_attr(unode, name, "type", type);
        } else if (val && *val) {
            hdf_set_value(unode, name, val);
            mcs_set_int_attr(unode, name, "type", type);
        } else if (require && !strcmp(require, "true")) {
            err = nerr_raise(NERR_ASSERT, "需要 %s %d 参数", name, type);
            goto done;
        }

        cnode = hdf_obj_next(cnode);
    }

    err = mmg_hdf_update(db, dsn, flags, unode, qa);

done:
    SAFE_FREE(qa);
    hdf_destroy(&unode);

    return nerr_pass(err);
}

NEOERR* mmg_count(mmg_conn *db, char *dbname, char *collname, int *ret, char *querys)
{
    bson *doc;

    MCS_NOT_NULLC(db, dbname, collname);
    MCS_NOT_NULLB(querys, ret);

    mtc_noise("count %s.%s %s", dbname, collname, querys);

    doc = mbson_new_from_string(querys, true);
    if (!doc) return nerr_raise(NERR_ASSERT, "build doc: %s: %s",
                                querys, strerror(errno));

    *ret = (int)mongo_sync_cmd_count(db->con, dbname, collname, doc);

    bson_free(doc);

    return STATUS_OK;
}

NEOERR* mmg_countf(mmg_conn *db, char *dbname, char *collname, int *ret, char *qfmt, ...)
{
    char *qa;
    va_list ap;
    NEOERR *err;

    va_start(ap, qfmt);
    qa = vsprintf_alloc(qfmt, ap);
    va_end(ap);
    if (!qa) return nerr_raise(NERR_NOMEM, "Unable to allocate mem for string");

    err = mmg_count(db, dbname, collname, ret, qa);
    if (err != STATUS_OK) return nerr_pass(err);

    free(qa);

    return STATUS_OK;
}

NEOERR* mmg_delete(mmg_conn *db, char *dsn, int flags, char *sel)
{
    bson *doc;

    MCS_NOT_NULLC(db, dsn, sel);

    mtc_noise("delete %s %s", dsn, sel);

    doc = mbson_new_from_string(sel, true);
    if (!doc) return nerr_raise(NERR_ASSERT, "build doc: %s: %s",
                                sel, strerror(errno));

    if (!mongo_sync_cmd_delete(db->con, dsn, flags, doc)) {
        GET_LAST_ERROR(db->con, dsn);
        return nerr_raise(NERR_DB, "sync_cmd_delete: %s %d %s",
                          m_errmsg, errno, strerror(errno));
    }

    bson_free(doc);

    return STATUS_OK;
}

NEOERR* mmg_deletef(mmg_conn *db, char *dsn, int flags, char *selfmt, ...)
{
    char *qa;
    va_list ap;
    NEOERR *err;

    va_start(ap, selfmt);
    qa = vsprintf_alloc(selfmt, ap);
    va_end(ap);
    if (!qa) return nerr_raise(NERR_NOMEM, "Unable to allocate mem for string");

    err = mmg_delete(db, dsn, flags, qa);
    if (err != STATUS_OK) return nerr_pass(err);

    free(qa);

    return STATUS_OK;
}

NEOERR* mmg_custom(mmg_conn *db, char *dbname,
                   char *prefix, HDF *outnode, char *command)
{
    mongo_packet *p;
    mongo_sync_cursor *c;
    bson *doc;

    NEOERR *err;

    MCS_NOT_NULLC(db, dbname, command);

    mtc_noise("custom %s %s", dbname, command);

    /*
     * doc
     */
    doc = mbson_new_from_string(command, true);
    if (!doc) return nerr_raise(NERR_ASSERT, "build doc custom %s", command);

    p = mongo_sync_cmd_custom(db->con, dbname, doc);
    bson_free(doc);
    if (!p) {
        GET_LAST_ERROR(db->con, dbname);
        return nerr_raise(NERR_DB, "sync_cmd_custom: %s %s %d %s",
                          command, m_errmsg, errno, strerror(errno));
    }

    c = mongo_sync_cursor_new(db->con, dbname, p);
    if (!c) return nerr_raise(NERR_DB, "cursor: %s", strerror(errno));

    if (outnode) {
        if (mongo_sync_cursor_next(c)) {
            doc = mongo_sync_cursor_get_data(c);
            if (!doc) return nerr_raise(NERR_DB, "doc: %s", strerror(errno));

            err = mbson_export_to_hdf(outnode, doc, prefix, MBSON_EXPORT_TYPE, true);
            if (err != STATUS_OK) return nerr_pass(err);
        } else return nerr_raise(NERR_DB, "cursor next: %s", strerror(errno));
    }

    mongo_sync_cursor_free(c);

    return STATUS_OK;
}

NEOERR* mmg_customf(mmg_conn *db, char *dbname,
                    char *prefix, HDF *outnode, char *cmdfmt, ...)
{
    char *qa;
    va_list ap;
    NEOERR *err;

    va_start(ap, cmdfmt);
    qa = vsprintf_alloc(cmdfmt, ap);
    va_end(ap);
    if (!qa) return nerr_raise(NERR_NOMEM, "Unable to allocate mem for string");

    err = mmg_custom(db, dbname, prefix, outnode, qa);
    if (err != STATUS_OK) return nerr_pass(err);

    free(qa);

    return STATUS_OK;
}



char* mmg_get_valuef(mmg_conn *db, char *dsn, char *key, int skip, char *qfmt, ...)
{
    HDF *tmpnode; hdf_init(&tmpnode);
    char *val, *querys, sels[256];
    va_list ap;
    NEOERR *err;

    va_start(ap, qfmt);
    querys = vsprintf_alloc(qfmt, ap);
    va_end(ap);
    if (!querys) {
        mtc_err("Unable to allocate mem for query string");
        return NULL;
    }

    snprintf(sels, sizeof(sels), "{'%s': 1}", key);
    err = mmg_prepare(db, MMG_FLAG_EMPTY, skip, 1, NULL, sels, querys);
    RETURN_V_NOK(err, NULL);

    err = mmg_query(db, dsn, NULL, tmpnode);
    RETURN_V_NOK(err, NULL);

    err = hdf_get_copy(tmpnode, key, &val, NULL);
    RETURN_V_NOK(err, NULL);

    hdf_destroy(&tmpnode);
    SAFE_FREE(querys);

    return val;
}

int mmg_get_int_valuef(mmg_conn *db, char *dsn, char *key, int skip, int limit,
                       char *qfmt, ...)
{
    HDF *tmpnode; hdf_init(&tmpnode);
    char *querys, sels[256];
    int val;
    va_list ap;
    HDF *node;
    NEOERR *err;

    va_start(ap, qfmt);
    querys = vsprintf_alloc(qfmt, ap);
    va_end(ap);
    if (!querys) {
        mtc_err("Unable to allocate mem for query string");
        return 0;
    }

    snprintf(sels, sizeof(sels), "{'%s': 1}", key);
    err = mmg_prepare(db, MMG_FLAG_EMPTY, skip, limit, NULL, sels, querys);
    RETURN_V_NOK(err, 0);

    err = mmg_query(db, dsn, NULL, tmpnode);
    RETURN_V_NOK(err, 0);

    val = 0;

    node = hdf_obj_child(tmpnode);
    if (!node) {
        val = hdf_get_int_value(tmpnode, key, 0);
    }

    while (node) {
        val += hdf_get_int_value(node, key, 0);

        node = hdf_obj_next(node);
    }

    hdf_destroy(&tmpnode);
    SAFE_FREE(querys);

    return val;
}
