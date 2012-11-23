
/* Global data used throughout the whole application. */

#ifndef _COMMON_H
#define _COMMON_H

/* The queue for database operations */
#include "queue.h"
#include "meventd.h"

#define REQ_MAKESURE_PARAM(hdf, key)                                \
    do {                                                            \
        if (!hdf_get_value(hdf, key, NULL))                         \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
    } while (0)

#define REQ_GET_PARAM_INT(hdf, key, ret)                            \
    do {                                                            \
        if (!hdf_get_value(hdf, key, NULL)) {                       \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
        }                                                           \
        ret = hdf_get_int_value(hdf, key, 0);                       \
    } while (0)

#define REQ_GET_PARAM_LONG(hdf, key, ret)                           \
    do {                                                            \
        if (!hdf_get_value(hdf, key, NULL)) {                       \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
        }                                                           \
        ret = strtoul(hdf_get_value(hdf, key, NULL), NULL, 10);     \
    } while (0)

#define REQ_GET_PARAM_STR(hdf, key, ret)                            \
    do {                                                            \
        ret = hdf_get_value(hdf, key, NULL);                        \
        if (!ret || *ret == '\0') {                                 \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
        }                                                           \
    } while (0)

#define REQ_GET_PARAM_OBJ(hdf, key, ret)                            \
    do {                                                            \
        ret = hdf_get_obj(hdf, key);                                \
        if (!ret) {                                                 \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
        }                                                           \
    } while (0)

#define REQ_GET_PARAM_CHILD(hdf, key, ret)                          \
    do {                                                            \
        ret = hdf_get_child(hdf, key);                              \
        if (!ret) {                                                 \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
        }                                                           \
    } while (0)


#define REQ_FETCH_PARAM_INT(hdf, key, ret)          \
    do {                                            \
        ret = 0;                                    \
        if (hdf_get_value(hdf, key, NULL)) {        \
            ret = hdf_get_int_value(hdf, key, 0);   \
        }                                           \
    } while (0)

#define REQ_FETCH_PARAM_LONG(hdf, key, ret)                         \
    do {                                                            \
        ret = 0;                                                    \
        if (hdf_get_value(hdf, key, NULL)) {                        \
            ret = strtoul(hdf_get_value(hdf, key, NULL), NULL, 10); \
        }                                                           \
    } while (0)

#define REQ_FETCH_PARAM_STR(hdf, key, ret)                  \
    do {                                                    \
        ret = hdf_get_value(hdf, key, NULL);                \
    } while (0)

#define REQ_FETCH_PARAM_OBJ(hdf, key, ret)      \
    do {                                        \
        ret = hdf_get_obj(hdf, key);            \
    } while (0)

extern struct mevent *mevent;

/* Settings */
struct settings {
    int tipc_lower;
    int tipc_upper;
    char *tcp_addr;
    int tcp_port;
    char *udp_addr;
    int udp_port;
    char *sctp_addr;
    int sctp_port;
    int foreground;
    int passive;
    char *logfname;
    int trace_level;
    char *conffname;
    int smsalarm;
};
extern struct settings settings;

/* Statistics */
#include "stats.h"
extern struct stats stats;

#include "ClearSilver.h"
extern HDF *g_cfg;

void clock_handler(const int fd, const short which, void *arg);
size_t explode(const char split, char *input, char **tP, unsigned int limit);

#endif

