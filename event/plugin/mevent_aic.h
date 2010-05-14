#ifndef __MEVENT_AIC_H__
#define __MEVENT_AIC_H__

#define PREFIX_AIC		"Appinfo"

enum {
	REQ_CMD_APPINFO = 1001,
	REQ_CMD_APPNEW,
	REQ_CMD_UPAPP
} req_cmd_aic;

enum {
	REP_OK_INSERT = 0x65,	/* 101 */
	REP_OK_UPDATE,
	REP_ERR_ALREADYBLACK = 0x401, 	/* 1025 */
	REP_ERR_ALREADYSTRANGER
} rep_code_aic;

#endif	/* __MEVENT_AIC_H__ */
