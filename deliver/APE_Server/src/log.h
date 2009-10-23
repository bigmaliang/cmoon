
#ifndef _LOG_H
#define _LOG_H

/* Maximum string to log */
#define MAX_LOG_STR 1024

#define	TC_DIE		0
#define TC_FOO		1
#define	TC_ERROR	2
#define	TC_WARNING	3	/* warning's pri is higher than debug. */
#define	TC_INFO		4
#define	TC_DEBUG	5
#define TC_NOISE	6

#define TC_LEVELS	7

int log_init(char *logfname);
int log_reopen(char *logfname);
void log_setlv(int lv);
void log_done();


#define wlog_die(f,...)													\
	do {																\
		wlog(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DIE,f,##__VA_ARGS__); \
		exit(-1);														\
	} while(0)
#define wlog_foo(f,...)		wlog(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_FOO,f,##__VA_ARGS__)
#define wlog_err(f,...)		wlog(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_ERROR,f,##__VA_ARGS__)
#define wlog_warn(f,...)	wlog(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_WARNING,f,##__VA_ARGS__)
#define wlog_dbg(f,...)		wlog(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DEBUG,f,##__VA_ARGS__)
#define wlog_info(f,...)	wlog(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_INFO,f,##__VA_ARGS__)
#define wlog_noise(f,...)	wlog(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_NOISE,f,##__VA_ARGS__)

#define wlog_errlog(s)		wlog(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_ERROR,"%s: %s",s,strerror(errno))


/* Normal logging, printf()-alike */
void wlog(const char *func, const char *file, long line,
		  int level, const char *fmt, ...);

#endif


