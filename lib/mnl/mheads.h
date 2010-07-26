#ifndef __MHEADS_H__
#define __MHEADS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>			/* strcasecmp() */
#include <stdbool.h>
#include <time.h>
#include <ctype.h>				/* isdigit() */
#include <inttypes.h>
#include <stdint.h>             /* uint32_t... */
#include <sys/types.h>
#include <dlfcn.h>				/* dlope()... */
#include <dirent.h>				/* scandir()... */

#include <stdarg.h>
#include <sys/stat.h>
#include <sys/shm.h>

/* includes for cs, memc */
#include "ClearSilver.h"
#include "libmemcached/memcached.h"
#ifndef DROP_MEVENT
#include "mevent.h"
#include "cache.h"
#endif
#ifndef DROP_FCGI
#include "fcgi_stdio.h"
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
 * #define mtc_err(X)					\
 * 		mtc_set_info(__FILE__, __LINE__, TC_ERROR);	\
 * 		mtc_msg X
 */
#endif

/* For compilers (well, cpp actually) which don't define __PRETTY_FUNCTION__ */
#ifndef __GNUC__
#define __PRETTY_FUNCTION__ "unknown_function"
#endif

/* includes for mnl */
#include "json.h"
#include "mnum.h"
#include "mcfg.h"		/* cfg.h must be the first one */
#include "mtypes.h"
#include "mconfig.h"
#include "mtrace.h"
#include "mcs.h"
#include "mmemc.h"
#include "mjson.h"
#include "mutil.h"
#include "md5.h"
#include "mdb.h"
#include "mdata.h"
#include "fdb.h"
#include "mmisc.h"
#include "mglobal.h"

#endif	/* __MHEADS_H__ */
