#ifndef __FDB_H__
#define __FDB_H__

#include <stdbool.h>
#include "mysql.h"
#include "ClearSilver.h"

#define LEN_SQL        1024

#define RET_DBOP_OK            0
#define RET_DBOP_INITE        11
#define RET_DBOP_CONNECTE    12
#define RET_DBOP_INPUTE        13
#define RET_DBOP_HDFNINIT    21
#define RET_DBOP_DBNINIT    31
#define RET_DBOP_DBINTRANS    32
#define RET_DBOP_SELECTE    41
#define RET_DBOP_INSERTE    42
#define RET_DBOP_UPDATEE    43

#define RET_DBOP_EXIST        51
#define RET_DBOP_NEXIST        52

#define RET_DBOP_MEMALLOCE    100

#define RET_DBOP_ERROR        1023
#define RET_DBOP_MAX_M        1024


typedef struct {
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char sql[LEN_SQL];
}fdb_t;

int fdb_init_long(fdb_t **fdb, char *ip, char *user, char *pass,
                  char *name, unsigned int port);
void fdb_free(fdb_t **fdb);
/* remember free result */
char *fdb_escape_string(fdb_t *fdb, const char *str);
int fdb_exec(fdb_t *fdb);
int fdb_fetch_row(fdb_t *fdb);
int fdb_affect_rows(fdb_t *fdb);
int fdb_get_last_id(fdb_t *fdb);
char* fdb_error(fdb_t *fdb);

void fdb_opfinish(int ret, HDF *hdf, fdb_t *fdb,
                  char *target, char *url, bool header);
void fdb_opfinish_json(int ret, HDF *hdf, fdb_t *fdb);

#endif    /* __FDB_H__ */
