#ifndef __MEVENT_PLUGIN_H__
#define __MEVENT_PLUGIN_H__

#include <sys/types.h>        /* for size_t */
#include <stdint.h>        /* for [u]int*_t */
#include <stdbool.h>        /* bool */
#include <stdlib.h>        /* for malloc() */
#include <string.h>        /* for memcpy()/memcmp() */
#include <stdio.h>        /* snprintf() */
#include <unistd.h>        /* usleep() */
#include <arpa/inet.h>    /* htonl() */
#include "mevent.h"
#include "net-const.h"
#include "common.h"
#include "packet.h"
#include "log.h"
#include "queue.h"
#include "cache.h"
#include "reply.h"
#include "smsalarm.h"
#include "syscmd.h"

#include "ClearSilver.h"

#include "mheads.h"

#define CACHE_HDF(hdf, timeout, fmt, ...)                               \
    if ((val = calloc(1, MAX_PACKET_LEN)) == NULL)                      \
        return nerr_raise(REP_ERR_MEM, "alloc mem for cache failure");  \
    vsize = pack_hdf(hdf, val, MAX_PACKET_LEN);                         \
    cache_setf(cd, val, vsize, timeout, fmt, ##__VA_ARGS__);            \
    free(val);

#define CACHE_SET_INT64(cd, num, timeout, fmt, ...)                 \
    do {                                                            \
        vsize = 24;                                                 \
        val = calloc(1, vsize);                                     \
        snprintf((char*)val, vsize, "%23lld", (long long int)num);  \
        cache_setf(cd, val, vsize, timeout, fmt, ##__VA_ARGS__);    \
        free(val);                                                  \
    } while (0)

#define CACHE_SET_INT(cd, num, timeout, fmt, ...)                   \
    do {                                                            \
        vsize = 12;                                                 \
        val = calloc(1, vsize);                                     \
        snprintf((char*)val, vsize, "%d", num);                     \
        cache_setf(cd, val, vsize, timeout, fmt, ##__VA_ARGS__);    \
        free(val);                                                  \
    } while (0)

#define TRACE_ERR(q, ret, err)                                          \
    do {                                                                \
        /* trace */                                                     \
        STRING zstra;    string_init(&zstra);                           \
        nerr_error_traceback(err, &zstra);                              \
        mtc_err("pro %u failed %d %s", q->operation, ret, zstra.buf);   \
        /* set to hdfsnd */                                             \
        hdf_set_value(q->hdfsnd, PRE_ERRTRACE, zstra.buf);              \
        string_clear(&zstra);                                           \
        NEOERR *neede = mcs_err_valid(err);                             \
        mcs_set_int_value_with_type(q->hdfsnd, PRE_ERRCODE, neede->error, CNODE_TYPE_INT); \
        hdf_set_value(q->hdfsnd, PRE_ERRMSG, neede->desc);              \
        nerr_ignore(&err);                                              \
    } while (0)

#endif    /* __MEVENT_PLUGIN_H__ */
