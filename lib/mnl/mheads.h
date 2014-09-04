#ifndef __MHEADS_H__
#define __MHEADS_H__

#include "mbase.h"

#include "ClearSilver.h"
#include "libmemcached/memcached.h"
#include "json.h"

#ifdef USE_FASTCGI
#include "fcgi_stdio.h"
#endif
#include "mkdio.h"
#include "gd.h"
#include "gdfontt.h"
#include "gdfonts.h"
#include "gdfontmb.h"
#include "gdfontl.h"
#include "gdfontg.h"

#include "mglobal.h"
#include "mtimer.h"
#include "mcfg.h"
#include "md5.h"
#include "mb64.h"
#include "mmkd.h"
#include "mcs.h"
#include "mstr.h"
#include "mutil.h"
#include "mtemplate.h"
#include "mfile.h"
#include "mimg.h"

#include "mtrace.h"
#include "mmemc.h"
#include "mjson.h"
#include "mhttp.h"
#include "mdb.h"

#ifndef DROP_MONGO
#include "mongo.h"
#include "mmg.h"
#include "mbson.h"
#endif

#endif    /* __MHEADS_H__ */
