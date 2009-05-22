
#ifndef _NET_CONST_H
#define _NET_CONST_H

/*
 * Local network constants.
 * Isolated so it's shared between the server and the library code.
 */

/* TIPC server type (hardcoded) and default instance. */
#define TIPC_SERVER_TYPE 26001
#define TIPC_SERVER_INST 10

/* TCP default listen address and port. */
#define TCP_SERVER_ADDR "0.0.0.0"
#define TCP_SERVER_PORT 26010

/* UDP default listen address and port. */
#define UDP_SERVER_ADDR "0.0.0.0"
#define UDP_SERVER_PORT 26010

/* SCTP default listen address and port. */
#define SCTP_SERVER_ADDR "0.0.0.0"
#define SCTP_SERVER_PORT 26010

#define TRACE_LEVEL	2
#define CONFIG_FILENAME "./mevent.hdf"

#define PRE_PLUGIN	"Plugin"
#define PRE_SERVER	"Mevent"
//#define PLUGIN_PATH	"/usr/local/findjb/idc/event/plugin/"
#define PLUGIN_PATH	"/usr/local/lib/"

/* Protocol version, for checking in the network header. */
#define PROTO_VER 1

/* Network requests */
enum {
	REQ_CMD_NONE = 0,
	REQ_CMD_GET,
	REQ_CMD_INSERT,
	REQ_CMD_SET,
	REQ_CMD_UPDATE,
	REQ_CMD_DELETE,
	REQ_CMD_INC,
	REQ_CMD_CAS,
	REQ_CMD_STATS
};


/* Possible request flags (which can be applied to the documented requests) */
#define FLAGS_NONE		0
#define FLAGS_CACHE_ONLY	1	/* get, set, del, cas, incr */
#define FLAGS_SYNC		2	/* set, del */

/* Network replies (different namespace from requests) */
#define REP_ERR			0x800
#define REP_CACHE_HIT		0x801
#define REP_CACHE_MISS		0x802
#define REP_OK			0x803
#define REP_NOTIN		0x804
#define REP_NOMATCH		0x805

/* Network error replies */
#define ERR_VER			0x101	/* Version mismatch */
#define ERR_SEND		0x102	/* Error sending data */
#define ERR_BROKEN		0x103	/* Broken request */
#define ERR_UNKREQ		0x104	/* Unknown request */
#define ERR_MEM			0x105	/* Memory allocation error */
#define ERR_DB			0x106	/* Database error */
#define ERR_BUSY		0x107	/* queue full */

enum {
	DATA_TYPE_EOF = 0,
	DATA_TYPE_U32,
	DATA_TYPE_ULONG,
	DATA_TYPE_STRING,
	DATA_TYPE_ARRAY
};


#endif

