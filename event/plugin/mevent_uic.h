#ifndef __MEVENT_UIC_H__
#define __MEVENT_UIC_H__

#define MAX_INCEPT_NUM		100
#define DEFAULT_GROUP_ID	1

#define PREFIX_FRIEND		"Friend"
#define PREFIX_SETTING		"Setting"

enum {
	REQ_CMD_FRIENDLIST = 1001,
	REQ_CMD_ISFRIEND,
	REQ_CMD_APPLYFRIEND,
	REQ_CMD_REJECTAPPLY,
	REQ_CMD_CONFIRMFRIEND,
	REQ_CMD_DELFRIEND,
	REQ_CMD_ADDGROUP = 2001,
	REQ_CMD_MODGROUP,
	REQ_CMD_DELGROUP,
	REQ_CMD_MYSETTING = 3001
} req_cmd;

enum {
	REP_OK_INSERT = 0x65,			/* 101 */
	REP_OK_UPDATE,
	REP_OK_ISFRIEND,
	REP_OK_NOTFRIEND,
	REP_ERR_ALREADYAPPLYED = 0x401, /* 1025 */
	REP_ERR_ALREADYFRIEND,
	REP_ERR_NOTAPPLY,
	REP_ERR_NOTFRIEND,
	REP_ERR_MAXGROUPLIMIT = 0x801, /* 2049 */
	REP_ERR_GROUPNAMEEXIST,
	REP_ERR_GROUPREADONLY,
	REP_ERR_NOTOWNER
} rep_code;

#endif	/* __MEVENT_UIC_H__ */
