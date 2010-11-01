#ifndef __MEVENT_AUX_H__
#define __MEVENT_AUX_H__

#define PREFIX_CMTAPP		"Comment"
#define CMT_CC_SEC			60

enum cmt_state {
	CMT_ST_NORMAL = 0,
	CMT_ST_DEL
};

enum req_cmd_aux {
	REQ_CMD_CMT_GET = 1001,
	REQ_CMD_CMT_ADD,
	REQ_CMD_CMT_DEL,
	REQ_CMD_MAIL_ADD = 2001
};

enum req_code_aux {
	REP_ERR_CMTGET = 501,
	REP_OK_CMTGET = 1001
};

#endif	/* __MEVENT_AUX_H__ */
