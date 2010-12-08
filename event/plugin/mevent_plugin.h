#ifndef __MEVENT_PLUGIN_H__
#define __MEVENT_PLUGIN_H__

#include <sys/types.h>		/* for size_t */
#include <stdint.h>		/* for [u]int*_t */
#include <stdbool.h>		/* bool */
#include <stdlib.h>		/* for malloc() */
#include <string.h>		/* for memcpy()/memcmp() */
#include <stdio.h>		/* snprintf() */
#include <unistd.h>		/* usleep() */
#include <arpa/inet.h>	/* htonl() */
#include "mevent.h"
#include "net-const.h"
#include "common.h"
#include "packet.h"
#include "log.h"
#include "queue.h"
#include "cache.h"
#include "reply.h"
#include "smsalarm.h"
#include "syscmd.h"

#include "ClearSilver.h"

#include "mheads.h"

#define CACHE_HDF(hdf, timeout, fmt, ...)								\
	if ((val = calloc(1, MAX_PACKET_LEN)) == NULL)						\
		return nerr_raise(REP_ERR_MEM, "alloc mem for cache failure");	\
	vsize = pack_hdf(hdf, val, MAX_PACKET_LEN);							\
	cache_setf(cd, val, vsize, timeout, fmt, ##__VA_ARGS__);			\
	free(val);

#define TRACE_ERR(q, ret, err)										\
	STRING zstra;	string_init(&zstra);							\
	nerr_error_traceback(err, &zstra);								\
	mtc_err("pro %u failed %d %s", q->operation, ret, zstra.buf);	\
	hdf_set_value(q->hdfsnd, PRE_ERRTRACE, zstra.buf);				\
	string_clear(&zstra);											\
	nerr_ignore(&err);

#endif	/* __MEVENT_PLUGIN_H__ */
