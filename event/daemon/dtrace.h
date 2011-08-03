#ifndef __DTRACE_H__
#define __DTRACE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define    TC_DIE        0
#define    TC_ERROR    1
#define    TC_WARNING    2    /* warning's pri is higher than debug. */
#define    TC_DEBUG    3
#define    TC_INFO        4
#define TC_NOISE    5

#define TC_LEVELS    6

#define TC_ROOT        "/data/logs/"

#define TC_DEFAULT_LEVEL    2
#define TC_MAX_SIZE        (10*1024*1024)
#define TC_MAX_NUM        5

#define FOPEN    fopen
#define FCLOSE    fclose
#define    FPRINTF    fprintf
#define VFPRINTF vfprintf
#define FFLUSH    fflush
#define FSYNC    fsync

/* Technically, we could do this in configure and detect what their compiler
 * can handle, but for now... */
#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define USE_C99_VARARG_MACROS 1
#elif __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 4) || defined (S_SPLINT_S)
#define USE_GNUC_VARARG_MACROS 1
#else
#error The compiler is missing support for variable-argument macros.
/*
 * or, we can use following form
 *
 * #define mtc_err(X)                    \
 *         mtc_set_info(__FILE__, __LINE__, TC_ERROR);    \
 *         mtc_msg X
 */
#endif

#if defined(USE_C99_VARARG_MACROS)
#define dtc_die(p,f,...)                                                \
    do {                                                                \
        dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DIE,f,##__VA_ARGS__); \
        exit(-1);                                                        \
    } while(0)
#define dtc_err(p,f,...)    dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_ERROR,f,##__VA_ARGS__)
#define dtc_warn(p,f,...)    dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_WARNING,f,##__VA_ARGS__)
#define dtc_dbg(p,f,...)    dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DEBUG,f,##__VA_ARGS__)
#define dtc_info(p,f,...)    dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_INFO,f,##__VA_ARGS__)
#define dtc_noise(p,f,...)    dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_NOISE,f,##__VA_ARGS__)
    
#elif defined(USE_GNUC_VARARG_MACROS)
#define dtc_die(p,f,a...)                                                \
    do {                                                                \
        dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DIE,f,##a);    \
        exit(-1);                                                        \
    } while(0)
#define dtc_err(p,f,a...)    dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_ERROR,f,##a)
#define dtc_warn(p,f,a...)    dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_WARNING,f,##a)
#define dtc_dbg(p,f,a...)    dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_DEBUG,f,##a)
#define dtc_info(p,f,a...)    dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_INFO,f,##a)
#define dtc_noise(p,f,a...)    dtc_msg(p,__PRETTY_FUNCTION__,__FILE__,__LINE__,TC_NOISE,f,##a)
#endif

FILE* dtc_init(const char *fn);
void dtc_leave(FILE *fp);
bool dtc_msg(FILE *fp, const char *func, const char *file, long line,
             int level, const char *format, ...);

#endif    /* __DTRACE_H__ */
