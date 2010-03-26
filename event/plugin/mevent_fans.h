#ifndef __MEVENT_FANS_H__
#define __MEVENT_FANS_H__

#define PREFIX_FANS		"Fans"
#define PREFIX_IDOLS	"Idols"

enum {
	FOLLOW_TYPE_FANS = 1,
	FOLLOW_TYPE_IDOL,
	FOLLOW_TYPE_FANSEO		/* followed each other */
} follow_type;

enum {
	REQ_CMD_FANSLIST = 1001,
	REQ_CMD_IDOLLIST,
	REQ_CMD_FOLLOWTYPE,
	REQ_CMD_FOLLOW,
	REQ_CMD_UNFOLLOW
} req_cmd_fans;

enum {
	REP_OK_CLEAN = 0x65,	/* 101 */
	REP_OK_ISFANS,
	REP_OK_ISIDOL,
	REP_OK_ISFANSEO,
	REP_ERR_ALREADYFANS = 0x401, /* 1025 */
	REP_ERR_NOTFANS
} rep_code_fans;

#endif	/* __MEVENT_FANS_H__ */
