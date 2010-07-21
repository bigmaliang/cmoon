#ifndef __MEVENT_AIC_H__
#define __MEVENT_AIC_H__

#define PREFIX_AIC		"Appinfo"
#define PREFIX_USERLIST	"AppUserlist"

#define LCS_TUNE_QUIET	0x01
#define LCS_TUNE_SMS	0x02

enum {
	LCS_ST_BLACK = 0,
	LCS_ST_STRANGER,
	LCS_ST_FREE = 10,			/* 20 online, 20 history raw, 2 admin */
	LCS_ST_VIPED,				/* ED history raw */
	LCS_ST_VIP,					/* 200 online, one year history raw, 20 admin */
	LCS_ST_VVIP					/* unlimit online, history raw, admin */
} lcsStat;

enum {
	REQ_CMD_APPINFO = 1001,
	REQ_CMD_APPNEW,
	REQ_CMD_APPUP,
	REQ_CMD_USERLIST = 2001,
	REQ_CMD_USERJOIN
} req_cmd_aic;

enum {
	REP_ERR_ALREADYREGIST = 501,
	REP_ERR_NREGIST,
	REP_ERR_ALREADYJOIN = 511,
	REP_OK_INSERT = 1001,
	REP_OK_UPDATE,
} rep_code_aic;

#endif	/* __MEVENT_AIC_H__ */
