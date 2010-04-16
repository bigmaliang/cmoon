
#ifndef _SMSALARM_H
#define _SMSALARM_H

#define SMSA_CFG_PATH	"Mevent.smsalarm"

#define SMSA_CFG_IP		hdf_get_value(g_cfg, SMSA_CFG_PATH".db.ip", "127.0.0.1")
#define SMSA_CFG_USER	hdf_get_value(g_cfg, SMSA_CFG_PATH".db.user", "test")
#define SMSA_CFG_PASS	hdf_get_value(g_cfg, SMSA_CFG_PATH".db.pass", "test")
#define SMSA_CFG_NAME	hdf_get_value(g_cfg, SMSA_CFG_PATH".db.name", "monitordb")
#define SMSA_CFG_PORT	hdf_get_int_value(g_cfg, SMSA_CFG_PATH".db.port", 3306)

#define SMS_ALARM(fmt, ...)							\
	do {											\
		if (settings.smsalarm) {					\
			smsalarm_msgf(fmt, ##__VA_ARGS__);		\
		}											\
	} while(0)

void smsalarm_msg(char *msg);
void smsalarm_msgf(char *fmt, ...);

#endif


