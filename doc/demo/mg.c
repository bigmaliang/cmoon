#include "mheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv, char **envp)
{
    struct stx {
        int state;
        char s[100];
        struct stx *x[3];
    };


    printf("%ld\n", sizeof(struct stx));

    return 0;
#if 0

    NEOERR *err;
    mmg_conn *db;

    err = mmg_init("172.10.7.211", 27017, &db);
    OUTPUT_NOK(err);
    
    err = mmg_string_update(db, "ml.count", "{'foo': 'bar'}", "{'foo': 'barr', 'xxx': 1}");
    OUTPUT_NOK(err);

    char *error = NULL;
    //mongo_sync_cmd_get_last_error(db->con, "ml.count", &error);
    printf("update error %d, %s", 1<<2, error);

    mmg_destroy(db);


#endif
    return 0;
}
