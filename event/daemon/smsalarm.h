
#ifndef _SMSALARM_H
#define _SMSALARM_H

#define SMSA_CFG_PATH    "Mevent.smsalarm"

#define SMSA_DB_SN        hdf_get_value(g_cfg, SMSA_CFG_PATH".dbsn", NULL)

#define SMS_ALARM(fmt, ...)                            \
    do {                                            \
        if (settings.smsalarm) {                    \
            smsalarm_msgf(fmt, ##__VA_ARGS__);        \
        }                                            \
    } while(0)

void smsalarm_msg(char *msg);
void smsalarm_msgf(char *fmt, ...);

#endif


