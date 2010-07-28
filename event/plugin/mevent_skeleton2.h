#ifndef __MEVENT_SKELETON2_H__
#define __MEVENT_SKELETON2_H__

#define PREFIX_SKLE		"Skele"

enum {
	REQ_CMD_SKGET = 1001,
	REQ_CMD_SKSET
} req_cmd_skeleton2;

enum {
	REP_ERR_SKGET = 501,
	REP_OK_SKGET = 1001
} rep_code_skeleton2;

#endif	/* __MEVENT_SKELETON2_H__ */
