#ifndef __MEVENT_MSG_H__
#define __MEVENT_MSG_H__

#define PREFIX_MYMSG		"Message"
#define PREFIX_MYSAID		"Mysaid"
#define PREFIX_SAYTOME		"Saytome"
#define PREFIX_SAYWITHOTHER	"Saywithother"
#define MSG_CC_SEC			60

enum msg_type {
	MSG_TYPE_SEND = 0,
	MSG_TYPE_OFFLINE_MSG,
	MSG_TYPE_JOIN,
	MSG_TYPE_VISIT,
	MSG_TYPE_LEFT,
	MSG_TYPE_UNKNOWN
};

enum req_cmd_msg {
	REQ_CMD_MYMSG = 1001,
	REQ_CMD_MYSAID,
	REQ_CMD_SAYTOME,
	REQ_CMD_SAYWITHOTHER,		/* TODO */
	REQ_CMD_MSGSET = 2001
};

enum req_code_msg {
	REP_ERR_SKGET = 501,
	REP_OK_SKGET = 1001
};

#endif	/* __MEVENT_MSG_H__ */
