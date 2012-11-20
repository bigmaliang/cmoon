#include "mheads.h"

NEOERR* mmg_init(char *host, int port, mmg_conn **db)
{
    if (!host) return nerr_raise(NERR_ASSERT, "paramter null");

    mmg_conn *ldb;
    
    *db = NULL;
    ldb = calloc(1, sizeof(mmg_conn));
    if (!ldb) return nerr_raise(NERR_NOMEM, "calloc for connection");

    ldb->con = mongo_sync_connect(host, port, true);
    if (!ldb->con) return nerr_raise(NERR_DB, "sync connect: %s", strerror(errno));
    mongo_sync_conn_set_auto_reconnect(ldb->con, true);

    *db = ldb;

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

NEOERR* mmg_prepare(mmg_conn *db, char *querys, char *sels, int start, int skip, int limit,
                    NEOERR* (*qcbk)(mmg_conn *db, HDF *node), int flag)
{
    if (!db || !querys) return nerr_raise(NERR_ASSERT, "paramter null");

    mtc_dbg("prepare %s %s", querys, sels);
    
    /*
     * doc query
     */
    if (db->docq) {
        bson_free(db->docq);
        db->docq = NULL;
    }

    if (flag & MMG_PRE_WHERE) {
        db->docq = bson_new();
        bson_append_string(db->docq, "$where", querys, -1);
        bson_finish(db->docq);
    } else {
        db->docq = mbson_new_from_string(querys, true);
    }
    if (!db->docq) return nerr_raise(NERR_ASSERT, "build query: %s", strerror(errno));


    /*
     * doc selector
     */
    if (db->docs) {
        bson_free(db->docs);
        db->docs = NULL;
    }
    if (sels) {
        db->docs = mbson_new_from_string(sels, true);
        if (!db->docs) return nerr_raise(NERR_ASSERT, "build selector: %s",
                                         strerror(errno));
    }

    /*
     * later mmg_prepare won't overwrite formal's callback
     */
    if (!db->incallback) db->query_callback = qcbk;
    db->start = start;
    db->skip  = skip;
    db->limit = limit;
    db->flag  = flag;
    db->rescount = 0;
    
    return STATUS_OK;
}

NEOERR* mmg_query(mmg_conn *db, char *dsn, char *prefix, HDF *node)
{
    int count;
    char key[LEN_HDF_KEY];
    HDF *cnode;
    bson *doc;
    NEOERR *err;
    
    MCS_NOT_NULLB(db, dsn);

    db->p = mongo_sync_cmd_query(db->con, dsn, db->start, db->skip, db->limit,
                                 db->docq, db->docs);
    if (!db->p) {
        if (errno == ENOENT) return STATUS_OK;
        return nerr_raise(NERR_DB, "query: %s %d", strerror(errno), errno);
    }

    /*
     * need get result
     */
    if (node) {
        db->c = mongo_sync_cursor_new(db->con, dsn, db->p);
        if (!db->c) return nerr_raise(NERR_DB, "cursor: %s", strerror(errno));

        cnode = NULL;
        count = 0;
        while (mongo_sync_cursor_next(db->c) && count < db->limit) {
            memset(key, sizeof(key), 0x0);
            
            if (prefix) {
                if (db->limit > 1) snprintf(key, sizeof(key), "%s.%d", prefix, count);
                else snprintf(key, sizeof(key), "%s", prefix);
            } else {
                if (db->limit > 1) sprintf(key, "%d", count);
                else key[0] = '\0';
            }
            
            doc = mongo_sync_cursor_get_data(db->c);
            err = mbson_export_to_hdf(node, doc, key, MBSON_EXPORT_TYPE, true);
            if (err != STATUS_OK) return nerr_pass(err);

            if (!cnode) cnode = hdf_get_obj(node, key);
            count++;
        }
        db->rescount = count;

        mongo_sync_cursor_free(db->c);
        db->c = NULL;
        db->p = NULL;
        
        mtc_dbg("queried %d result", count);

        /*
         * call callback at last. because we don't want declare more mmg_conn*
         * it's safe to do new query in callback on result stored (db->c freeed)
         * we can call mmg_query() recursive, the callback won't.
         */
        if (db->query_callback && !db->incallback) {
            db->incallback = true;
            
            while (cnode) {
                err = db->query_callback(db, cnode);
                TRACE_NOK(err);
                
                cnode = hdf_obj_next(cnode);
            }
            
            db->incallback = false;
        }
    } else {
        /* don't need result */
        mongo_wire_packet_free(db->p);
        db->c = NULL;
        db->p = NULL;
    }

    return STATUS_OK;
}
