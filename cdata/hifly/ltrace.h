#ifndef __FTRACE_H__
#define __FTRACE_H__

#include "lheads.h"

#define	TC_DIE		0
#define	TC_ERROR	1
#define	TC_WARNING	2	/* warning's pri is higher than debug. */
#define	TC_DEBUG	3
#define	TC_INFO		4
#define TC_NOISE	5

#define TC_LEVELS	6

#define TC_DEFAULT_LEVEL	2
#define TC_CFGSTR	"log_level"
#define TC_MAX_SIZE	(10*1024*1024)
#define TC_MAX_NUM	5

//#ifdef _FCGI_STDIO_NOK
//#define FOPEN	FCGI_fopen
//#define FCLOSE	FCGI_fclose
//#define	FPRINTF	FCGI_fprintf
//#define VFPRINTF FCGI_vfprintf
//#else
#define FOPEN	fopen
#define FCLOSE	fclose
#define	FPRINTF	fprintf
#define VFPRINTF vfprintf
//#endif

#if defined(USE_C99_VARARG_MACROS)
#define ftc_die(f,...)													\
	do {																\
		ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DIE,f,__VA_ARGS__); \
		exit(-1);														\
	} while(0)
#define ftc_err(f,...)		ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_ERROR,f,__VA_ARGS__)
#define ftc_warn(f,...)		ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_WARNING,f,__VA_ARGS__)
#define ftc_dbg(f,...)		ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DEBUG,f,__VA_ARGS__)
#define ftc_info(f,...)		ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_INFO,f,__VA_ARGS__)
#define ftc_noise(f,...)	ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_NOISE,f,__VA_ARGS__)
	
#elif defined(USE_GNUC_VARARG_MACROS)
#define ftc_die(f,a...)													\
	do {																\
		ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DIE,f,##a);	\
		exit(-1);														\
	} while(0)
#define ftc_err(f,a...)		ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_ERROR,f,##a)
#define ftc_warn(f,a...)	ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_WARNING,f,##a)
#define ftc_dbg(f,a...)		ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DEBUG,f,##a)
#define ftc_info(f,a...)	ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_INFO,f,##a)
#define ftc_noise(f,a...)	ftc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_NOISE,f,##a)
#endif

void ftc_init(const char *fn);
void ftc_leave();
bool ftc_msg(const char *func, const char *file, long line,
			 int level, const char *format, ...);

#endif	/* __FTRACE_H__ */
