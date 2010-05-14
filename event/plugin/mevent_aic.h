#ifndef __MEVENT_AIC_H__
#define __MEVENT_AIC_H__

#define PREFIX_AIC		"Appinfo"

enum {
	LCS_ST_BLACK = 0,
	LCS_ST_STRANGER,
	LCS_ST_FREE = 10,			/* 20 online, 20 history raw, 2 admin */
	LCS_ST_VIPED,				/* ED history raw */
	LCS_ST_VIP,					/* 200 online, one year history raw, 20 admin */
	LCS_ST_VVIP					/* unlimit online, history raw, admin */
} lcsStat;

enum {
	LCS_SMS_CLOSE = 0,
	LCS_SMS_OPEN,
	LCS_ADM_OFFLINE,
	LCS_ADM_ONLINE
} lcsFlag;

enum {
	REQ_CMD_APPINFO = 1001,
	REQ_CMD_APPNEW,
	REQ_CMD_APPUP
} req_cmd_aic;

enum {
	REP_OK_INSERT = 0x65,	/* 101 */
	REP_OK_UPDATE,
	REP_ERR_ALREADYREGIST = 0x401, 	/* 1025 */
	REP_ERR_NREGIST
} rep_code_aic;

#endif	/* __MEVENT_AIC_H__ */
