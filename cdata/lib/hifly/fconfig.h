#ifndef __FCONFIG_H__
#define __FCONFIG_H__

#ifdef RELEASE
#define NMDB_SERVER_USER	"192.168.1.9"
#define NMDB_SERVER_PHOTO	"192.168.1.9"
#define NMDB_SERVER_VIDEO	"192.168.1.9"
#define NMDB_SERVER_BLOG	"192.168.1.9"
#define DB_IP			"192.168.1.24"
#else
#define NMDB_SERVER_USER	"192.168.8.54"
#define NMDB_SERVER_PHOTO	"192.168.8.54"
#define NMDB_SERVER_VIDEO	"192.168.8.54"
#define NMDB_SERVER_BLOG	"192.168.8.54"
#define DB_IP			"192.168.8.84"
#endif

#define NMDB_PORT_USER		26010
#define NMDB_PORT_PHOTO		26020
#define NMDB_PORT_VIDEO		26030
#define NMDB_PORT_BLOG		26040

#define DB_USER		"root"
#define DB_PASS		"hifly1234"

#define PRE_QUERY	"Query"
#define PRE_OUTPUT	"Output"
#define PRE_ERRMSG	PRE_OUTPUT".errmsg"
#define PRE_SUCCESS	PRE_OUTPUT".success"
#define POST_INCREMENT	"increment"
#define POST_TIMESTAMP	"timestamp"

#define LEN_INT		12
#define LEN_LONG	20
#define LEN_DT		11		// date length 2008-01-01
#define LEN_TM		25		// time 2008-01-01 12:12:43
#define LEN_TM_GMT	32		/* Wdy, DD-Mon-YYYY HH:MM:SS GMT */
#define LEN_LT		64
#define LEN_ST		128
#define LEN_SM		256
#define LEN_MD		1024
#define LEN_FN		256
#define LEN_SQL		1024

#define LEN_NMDB_KEY		256
#define LEN_NMDB_VAL		(1024*64)

#define HF_LOG_PATH	"/data/logs/cgi/"

enum query_method {
	QUERY_GET = 0,
	QUERY_POST,
	QUERY_PUT,
	QUERY_DEL,
	QUERY_UNKNOWN
};

#endif	/* __FCONFIG_H__ */
