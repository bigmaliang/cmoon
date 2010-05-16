#ifndef __MEVENT_DYN_H__
#define __MEVENT_DYN_H__

#define PREFIX_DYN		"Dynamic"

enum {
	REQ_CMD_JOINGET = 1001,
	REQ_CMD_JOINSET
} req_cmd_dyn;

enum {
	REP_ERR_DYN = 501,
	REP_OK_DYN = 1001
} rep_code_dyn;

#endif	/* __MEVENT_DYN_H__ */
