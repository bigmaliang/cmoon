#ifndef __MEVENT_DYN_H__
#define __MEVENT_DYN_H__

#define PREFIX_ADMIN		"Admin"

enum {
	TYPE_JOIN = 0,
	TYPE_VISIT
};

enum {
	REQ_CMD_ADDTRACK = 1001,
	REQ_CMD_GETADMIN
};

enum {
	REP_ERR_DYN = 501,
	REP_OK_DYN = 1001
};

#endif	/* __MEVENT_DYN_H__ */
