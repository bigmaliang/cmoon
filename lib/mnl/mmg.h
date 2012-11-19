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
    int start;
    int skip;
    int limit;
    int flag;
    int rescount;
} mmg_conn;

enum {
    MMG_PRE_NONE = 0,
    /* set querys to $where object instead of compile */
    MMG_PRE_WHERE = 1 << 1
};

NEOERR* mmg_init(char *host, int port, mmg_conn **db);
void mmg_destroy(mmg_conn *db);

NEOERR* mmg_prepare(mmg_conn *db, char *querys, char *sels, int start, int skip, int limit,
                    NEOERR* (*qcbk)(mmg_conn *db, HDF *node), int flag);

NEOERR* mmg_query(mmg_conn *db, char *dsn, char *prefix, HDF *node);

    
__END_DECLS
#endif    /* __MMG_H__ */
