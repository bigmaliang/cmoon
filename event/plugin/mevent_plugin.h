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
#include "data.h"
#include "packet.h"
#include "log.h"
#include "queue.h"
#include "cache.h"
#include "reply.h"
#include "smsalarm.h"
#include "syscmd.h"

#include "ClearSilver.h"

#include "mheads.h"

#endif	/* __MEVENT_PLUGIN_H__ */
