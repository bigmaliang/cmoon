
#ifndef _NET_CONST_H
#define _NET_CONST_H

/*
 * Local network constants.
 * Isolated so it's shared between the server and the library code.
 */

#define MAX_PACKET_LEN	(68*1024)
#define RESERVE_SIZE	512

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

#define TRACE_LEVEL	2
#define CONFIG_FILENAME "./mevent.hdf"

#define PRE_PLUGIN	"Plugin"
#define PRE_SERVER	"Mevent"
//#define PLUGIN_PATH	"/usr/local/findjb/idc/event/plugin/"
#define PLUGIN_PATH	"/usr/local/lib/"

/* Protocol version, for checking in the network header. */
#define PROTO_VER 1

/* Possible request flags (which can be applied to the documented requests) */
#define FLAGS_NONE		0
#define FLAGS_CACHE_ONLY 1	/* get, set, del, cas, incr */
#define FLAGS_SYNC		2	/* set, del */

#define PROCESS_OK(ret)	(ret >= REP_OK && ret < REP_ERR)
#define PROCESS_NOK(ret) (ret < REP_OK && ret >= REP_ERR)

#define REP_OK			0x64	/* 100 */

#define REP_ERR			0x200	/* 512 */
#define REP_ERR_VER		0x201
#define REP_ERR_SEND	0x202	/* Error sending data */
#define REP_ERR_BROKEN	0x203	/* Broken request */
#define REP_ERR_UNKREQ	0x204	/* Unknown request */
#define REP_ERR_MEM		0x205	/* Memory allocation error */
#define REP_ERR_DB		0x206	/* Database error */
#define REP_ERR_BUSY	0x207	/* queue full */
#define REP_ERR_PACK	0x208	/* packet data failure */
#define REP_ERR_BADPARAM 0x209	/* parameter error */

#define REP_ERR_APP		0x400	/* application error start point 1024 */

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

enum {
	DATA_TYPE_EOF = 0,
	DATA_TYPE_U32,
	DATA_TYPE_ULONG,
	DATA_TYPE_STRING,
	DATA_TYPE_ARRAY
};

#endif

