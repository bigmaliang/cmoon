#ifndef __MTRACE_H__
#define __MTRACE_H__

#include "mheads.h"

__BEGIN_DECLS

#define	TC_DIE		0
#define TC_FOO		1
#define	TC_ERROR	2
#define	TC_WARNING	3	/* warning's pri is higher than debug. */
#define	TC_DEBUG	4
#define	TC_INFO		5
#define TC_NOISE	6

#define TC_LEVELS	7

#define TC_DEFAULT_LEVEL	3
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
#define mtc_die(f,...)													\
	do {																\
		mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DIE,f,##__VA_ARGS__); \
		exit(-1);														\
	} while(0)
#define mtc_foo(f,...)		mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_FOO,f,##__VA_ARGS__)
#define mtc_err(f,...)		mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_ERROR,f,##__VA_ARGS__)
#define mtc_warn(f,...)		mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_WARNING,f,##__VA_ARGS__)
#define mtc_dbg(f,...)		mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DEBUG,f,##__VA_ARGS__)
#define mtc_info(f,...)		mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_INFO,f,##__VA_ARGS__)
#define mtc_noise(f,...)	mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_NOISE,f,##__VA_ARGS__)
	
#elif defined(USE_GNUC_VARARG_MACROS)
#define mtc_die(f,a...)													\
	do {																\
		mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DIE,f,##a);	\
		exit(-1);														\
	} while(0)
#define mtc_foo(f,a...)		mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_FOO,f,##a)
#define mtc_err(f,a...)		mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_ERROR,f,##a)
#define mtc_warn(f,a...)	mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_WARNING,f,##a)
#define mtc_dbg(f,a...)		mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DEBUG,f,##a)
#define mtc_info(f,a...)	mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_INFO,f,##a)
#define mtc_noise(f,a...)	mtc_msg(__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_NOISE,f,##a)
#endif

void mtc_init(const char *fn);
void mtc_leave();
bool mtc_msg(const char *func, const char *file, long line,
			 int level, const char *format, ...);

__END_DECLS
#endif	/* __MTRACE_H__ */
