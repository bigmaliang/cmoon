#ifndef __MEVENT_UDC_H__
#define __MEVENT_UDC_H__

#define PREFIX_BONUS	"Bonus"
#define PREFIX_EXP		"Experience"

enum {
	REQ_CMD_GETBONUS = 1001,
	REQ_CMD_SETBONUS,
	REQ_CMD_GETEXP,
	REQ_CMD_SETEXP
} req_cmd_udc;

enum {
	REP_ERR_NOBONUS = 0x401, 	/* 1025 */
	REP_ERR_NOEXP
} rep_code_udc;

#endif	/* __MEVENT_UDC_H__ */
