#ifndef __MBASE_H__
#define __MBASE_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>            /* strcasecmp() */
#include <stdbool.h>
#include <time.h>
#include <ctype.h>                /* isdigit() */
#include <inttypes.h>
#include <stdint.h>             /* uint32_t... */
#include <sys/types.h>
#include <sys/time.h>           /* gettimeofday() */
#include <dlfcn.h>                /* dlope()... */
#include <dirent.h>                /* scandir()... */
#include <iconv.h>

#include <stdarg.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/resource.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#endif

/* Fix Up for systems that don't define these standard things */
#ifndef __BEGIN_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif
#endif

#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX 255
#endif

#ifndef S_IXGRP
#define S_IXGRP S_IXUSR
#endif
#ifndef S_IWGRP
#define S_IWGRP S_IWUSR
#endif
#ifndef S_IRGRP
#define S_IRGRP S_IRUSR
#endif
#ifndef S_IXOTH
#define S_IXOTH S_IXUSR
#endif
#ifndef S_IWOTH
#define S_IWOTH S_IWUSR
#endif
#ifndef S_IROTH
#define S_IROTH S_IRUSR
#endif

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

/* For compilers (well, cpp actually) which don't define __PRETTY_FUNCTION__ */
#ifndef __GNUC__
#define __PRETTY_FUNCTION__ "unknown_function"
#endif

#endif  /* __MBASE_H__ */
