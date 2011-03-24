#ifndef __MEVENT_AIC_H__
#define __MEVENT_AIC_H__

#define PREFIX_APPINFO	"Appinfo"
#define PREFIX_SECY		"Secy"
#define PREFIX_USERLIST	"AppUserlist"
#define PREFIX_APPOUSER	"AppOuser"
#define AIC_CC_SEC		3600

#define LCS_TUNE_QUIET	0x01
#define LCS_TUNE_SMS	0x02
#define LCS_TUNE_SECY	0x04	/* default aname when every body offline */

enum {
	LCS_ST_BLACK = 0,
	LCS_ST_STRANGER,
	LCS_ST_FREE = 10,			/* 20 online, 20 history raw, 2 admin */
	LCS_ST_VIPED,				/* ED history raw */
	LCS_ST_VIP,					/* 200 online, one year history raw, 20 admin */
	LCS_ST_VVIP,				/* unlimit online, history raw, admin */
	LCS_ST_ADMIN,
	LCS_ST_ROOT = 1001
} lcsStat;

enum {
	REQ_CMD_APPINFO = 1001,
	REQ_CMD_APPNEW,
	REQ_CMD_APPUP,
	REQ_CMD_APPDEL,
	REQ_CMD_APP_GETSECY,
	REQ_CMD_APP_SETSECY,
	REQ_CMD_APPUSERS = 2001,	/* who visited my app */
	REQ_CMD_APPUSERIN,			/* remember sb visited my app */
	REQ_CMD_APPUSEROUT,			/* remove JiangYou boy */
	REQ_CMD_APPUSERUP,
	REQ_CMD_APP_O_USERS = 3001,	/* who is my stuff */
	REQ_CMD_APP_GETRLINK = 4001,
 	REQ_CMD_APP_SETRLINK
} req_cmd_aic;

enum {
	REP_ERR_NREGIST = 36,
	REP_ERR_ALREADYREGIST,
	REP_ERR_MISSEMAIL,
	REP_ERR_NRESET,
	REP_ERR_NOTJOIN = 41,
	REP_ERR_ALREADYJOIN,
} rep_code_aic;

#endif	/* __MEVENT_AIC_H__ */
