#ifndef __MEVENT_APEEXT_H__
#define __MEVENT_APEEXT_H__

#include <stdbool.h>

enum {
	REQ_CMD_USERON  = 1001,
	REQ_CMD_USEROFF = 1002,
	REQ_CMD_MSGSND  = 2001,
	REQ_CMD_MSGBRD  = 2002,
	REQ_CMD_CONNECT = 3001,		/* called before CMD_CONNECT, for load balance. */
	REQ_CMD_HB      = 4001,		/* x send to v, keep heart beat */
	REQ_CMD_STATE = 9001
} req_cmd_aic;

enum {
	REP_ERR_ALLDIE = 25,
} rep_code_aic;

void ext_snake_sort();

#endif	/* __MEVENT_APEEXT_H__ */
