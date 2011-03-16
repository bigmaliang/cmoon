#ifndef __APEV_H__
#define __APEV_H__

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <event.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>		/* htonls() and friends */
#include <netinet/in.h>		/* INET stuff */
#include <stdint.h>		/* uint32_t and friends */

#include "ClearSilver.h"

#include "udps.h"
#include "req.h"
#include "parse.h"
#include "packet.h"
#include "net-const.h"
#include "mevent_ape_ext.h"

#define REQ_GET_PARAM_INT(hdf, key, ret)							\
    do {															\
		if (!hdf_get_value(hdf, key, NULL)) {						\
			return nerr_raise(REP_ERR_BADPARAM, "need %s", key);	\
		}															\
		ret = hdf_get_int_value(hdf, key, 0);						\
    } while (0)

#define REQ_GET_PARAM_LONG(hdf, key, ret)							\
    do {															\
		if (!hdf_get_value(hdf, key, NULL)) {						\
			return nerr_raise(REP_ERR_BADPARAM, "need %s", key);	\
		}															\
		ret = strtoul(hdf_get_value(hdf, key, NULL), NULL, 10);		\
    } while (0)

#define REQ_GET_PARAM_STR(hdf, key, ret)							\
    do {															\
		ret = hdf_get_value(hdf, key, NULL);						\
		if (!ret || *ret == '\0') {									\
			return nerr_raise(REP_ERR_BADPARAM, "need %s", key);	\
		}															\
    } while (0)


#define REQ_FETCH_PARAM_INT(hdf, key, ret)			\
    do {											\
		if (hdf_get_value(hdf, key, NULL)) {		\
			ret = hdf_get_int_value(hdf, key, 0);	\
		}											\
    } while (0)

#define REQ_FETCH_PARAM_LONG(hdf, key, ret)							\
    do {															\
		if (hdf_get_value(hdf, key, NULL)) {						\
			ret = strtoul(hdf_get_value(hdf, key, NULL), NULL, 10); \
		}															\
    } while (0)

#define REQ_FETCH_PARAM_STR(hdf, key, ret)		\
    do {										\
		ret = hdf_get_value(hdf, key, NULL);	\
    } while (0)

#endif
