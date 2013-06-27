
#ifndef _NET_CONST_H
#define _NET_CONST_H

#include "ClearSilver.h"

/*
 * Local network constants.
 * Isolated so it's shared between the server and the library code.
 */

#define MAX_MEMPACK_LEN (6*1024*1024)
#define MAX_PACKET_LEN  (64*1024)
#define RESERVE_SIZE    512

/* TIPC server type (hardcoded) and default instance. */
#define TIPC_SERVER_TYPE 26001
#define TIPC_SERVER_INST 10

/* TCP default listen address and port. */
#define TCP_SERVER_ADDR "0.0.0.0"
#define TCP_SERVER_PORT 26000

/* UDP default listen address and port. */
#define UDP_SERVER_ADDR "0.0.0.0"
#define UDP_SERVER_PORT 26000

/* SCTP default listen address and port. */
#define SCTP_SERVER_ADDR "0.0.0.0"
#define SCTP_SERVER_PORT 26000

#define TRACE_LEVEL    2
#define CONFIG_FILENAME "./mevent.hdf"

#define PRE_PLUGIN    "Plugin"
#define PLUGIN_PATH    "/usr/local/lib/"

/* Protocol version, for checking in the network header. */
#define PROTO_VER 1

/* Possible request flags (which can be applied to the documented requests) */
#define FLAGS_NONE       0
#define FLAGS_CACHE_ONLY 1    /* get, set, del, cas, incr */
#define FLAGS_SYNC       2    /* set, del */

#define PROCESS_OK(ret)  (ret >= REP_OK)
#define PROCESS_NOK(ret) (ret < REP_OK)

enum {
    DATA_TYPE_EOF = 0,
    DATA_TYPE_U32,
    DATA_TYPE_ULONG,
    DATA_TYPE_STRING,
    DATA_TYPE_ARRAY,
    DATA_TYPE_ANY                /* used in data_cell_search, include all type */
};

/* Network requests */
enum {
    REQ_CMD_NONE = 0,
    REQ_CMD_CACHE_GET = 100,
    REQ_CMD_CACHE_SET,
    REQ_CMD_CACHE_DEL,
    REQ_CMD_CACHE_EMPTY,
    REQ_CMD_STATS = 1000        /* MAX system command is 1000 */
} req_cmd_sys;

extern NERR_TYPE REP_ERR;        /* 14 binded with neo_error, see pop/pub/lerr.c */
extern NERR_TYPE REP_ERR_VER;
extern NERR_TYPE REP_ERR_SEND;
extern NERR_TYPE REP_ERR_BROKEN;
extern NERR_TYPE REP_ERR_UNKREQ;
extern NERR_TYPE REP_ERR_MEM;
extern NERR_TYPE REP_ERR_DB;
extern NERR_TYPE REP_ERR_BUSY;
extern NERR_TYPE REP_ERR_PACK;
extern NERR_TYPE REP_ERR_BADPARAM;
extern NERR_TYPE REP_ERR_CACHE_MISS; /* 24 */

enum {
    REP_OK = 1000,                /* ok start point */
};

#endif

