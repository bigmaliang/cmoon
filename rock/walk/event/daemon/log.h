
#ifndef _LOG_H
#define _LOG_H

/* Maximum string to log */
#define MAX_LOG_STR 512

int log_init(void);
int log_reopen(void);

/* Normal logging, printf()-alike */
void wlog(const char *fmt, ...);

/* Errno logging, perror()-alike */
void errlog(const char *s);

#endif


