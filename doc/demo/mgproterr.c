#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ClearSilver.h>

#include <mongo.h>

#define DSN "test.foo"
#define TRYCOUNT 6
#define TIMEOUT 10

HDF *g_cfg = NULL;

void cmd_insert(mongo_sync_connection *con)
{
    bson *ba;
    
    ba = bson_build(BSON_TYPE_INT32, "xxx", 10, BSON_TYPE_NONE);
    bson_finish(ba);

    if (!mongo_sync_cmd_insert(con, DSN, ba, NULL)) {
        perror("sync_cmd_insert");
    }

    return;
}

void cmd_query(mongo_sync_connection *con)
{
    bson *ba;
    mongo_packet *p;
    mongo_sync_cursor *c;
    char *errmsg = NULL;
    
    ba = bson_build(BSON_TYPE_INT32, "xxx", 10, BSON_TYPE_NONE);
    bson_finish(ba);

    p = mongo_sync_cmd_query(con, DSN, 0, 0, 1, ba, NULL);
    if (!p) {
        mongo_sync_cmd_get_last_error(con, DSN, &errmsg);
        printf("%s", errmsg);
        perror("sync_cmd_query");
        free(errmsg);
        return;
    }

    c = mongo_sync_cursor_new(con, DSN, p);
    if (!c) {
        mongo_sync_cmd_get_last_error(con, DSN, &errmsg);
        printf("%s", errmsg);
        perror("sync_cursor_new");
        free(errmsg);
        return;
    }

    while(mongo_sync_cursor_next(c)) {
        bson *result = mongo_sync_cursor_get_data(c);
        bson_cursor *bc = bson_cursor_new(result);

        while (bson_cursor_next(bc)) {
            printf("key \t %s\n", bson_cursor_key(bc));
        }

        bson_cursor_free(bc);
        bson_free(result);
    }

    bson_free(ba);
    //mongo_wire_packet_free(p);
    mongo_sync_cursor_free(c);

    printf("query ok\n");
    
    return;
}

int main(int argc, char *argv[])
{
    mongo_sync_connection *con;
    int count;

    con = mongo_sync_connect("172.10.7.211", 27017, TRUE);
    if (!con) {
        perror("sync_connect");
        return 1;
    }

    mongo_connection_set_timeout(con, 400);

    printf("insert\n");
    cmd_insert(con);

    printf("query\n");
    for (int i = 0; i < TRYCOUNT; i++) {
        cmd_query(con);
    }

    printf("run use admin; db.fsyncLock() in mongoshell in %d secs, please\n", TIMEOUT);
    count = TIMEOUT;
    while (count--) {
        printf("in %d secs\n", count);
        sleep(1);
    }

    printf("query after fsyncLock()\n");
    for (int i = 0; i < TRYCOUNT; i++) {
        cmd_query(con);
    }

    printf("run db.fsyncUnlock() in mongoshell in %d secs, please\n", TIMEOUT);
    count = TIMEOUT;
    while (count--) {
        printf("in %d secs\n", count);
        sleep(1);
    }

    printf("query after fsyncUnlock()\n");
    for (int i = 0; i < TRYCOUNT; i++) {
        cmd_query(con);
    }

    mongo_sync_disconnect(con);
    return 0;
}
