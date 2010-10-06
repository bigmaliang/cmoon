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

#define DECLARE_CACHE()							\
	unsigned char *val = NULL;					\
	size_t vsize = 0;							\
	int hit;									\

#define CACHE_HDF(timeout, fmt, ...)									\
	if ((val = calloc(1, MAX_PACKET_LEN)) == NULL) return REP_ERR_MEM;	\
	vsize = pack_hdf(q->hdfsnd, val, MAX_PACKET_LEN);					\
	cache_setf(cd, val, vsize, timeout, fmt, ##__VA_ARGS__);			\
	free(val);

#endif	/* __MEVENT_PLUGIN_H__ */
