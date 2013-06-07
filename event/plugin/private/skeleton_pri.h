#ifndef __SKELETON_PRI_H__
#define __SKELETON_PRI_H__

#define PLUGIN_NAME    "skeleton"
#define CONFIG_PATH    PRE_SERVER"."PLUGIN_NAME

struct skeleton_stats {
    unsigned long msg_total;
    unsigned long msg_unrec;
    unsigned long msg_badparam;
    unsigned long msg_stats;

    unsigned long proc_suc;
    unsigned long proc_fai;
};

struct skeleton_entry {
    EventEntry base;
    mdb_conn *db;
    Cache *cd;
    struct skeleton_stats st;
};

#endif  /* __SKELETON_PRI_H__ */
