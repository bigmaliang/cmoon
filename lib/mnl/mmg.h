#ifndef __MMG_H__
#define __MMG_H__

#include "mheads.h"

__BEGIN_DECLS

typedef struct _mmg_conn {
    mongo_sync_connection *con;
    bson *docq;
    bson *docs;
    mongo_packet *p;
    mongo_sync_cursor *c;
    NEOERR* (*query_callback)(struct _mmg_conn *db, HDF *node);
    int flags;
    int skip;
    int limit;
    bool incallback;
    int rescount;
} mmg_conn;

NEOERR* mmg_init(char *host, int port, mmg_conn **db);
void mmg_destroy(mmg_conn *db);

/*
 * querys: query condition json object
 *         it can be just "{'obj_id': 10, 'user_id', 10}"
 *         or more complex like this:
 *         "{'$where': 'this.start_date+this.start_time > %ld'}"
 *         "{'$query': {'user_id': %d}, '$orderby': {'user_integral_id': 1}}"
 *
 * qcbk: the query callback on each result document.
 * ATTENTION: we can query recursive, but the callback won't call recursive.
 *            we just call the first NOTNULL qcbk
 *            because following prepare overwrite the formal's callback will coredump
 *            so, later prepare's callback won't be remember in db
 */
NEOERR* mmg_prepare(mmg_conn *db, char *querys, char *sels, int flags, int skip, int limit,
                    NEOERR* (*qcbk)(mmg_conn *db, HDF *node));

NEOERR* mmg_query(mmg_conn *db, char *dsn, char *prefix, HDF *outnode);

NEOERR* mmg_string_insert(mmg_conn *db, char *dsn, char *str);
NEOERR* mmg_hdf_insert(mmg_conn *db, char *dsn, HDF *node);

NEOERR* mmg_string_update(mmg_conn *db, char *dsn, char *sel, char *up);

NEOERR* mmg_count(mmg_conn *db, char *dbname, char *collname, char *querys, int *ret);

__END_DECLS
#endif    /* __MMG_H__ */
