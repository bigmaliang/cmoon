#ifndef __SYSCMD_H__
#define __SYSCMD_H__

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "net-const.h"
#include "queue.h"
#include "cache.h"
#include "data.h"
#include "packet.h"
#include "netutils.h"
#include "fdb.h"

#define VNAME_CACHE_KEY	"cachekey" 		/* DATA_TYPE_STRING */
#define VNAME_CACHE_VAL	"cacheval"      /* DATA_TYPE_ANY */

#define CASE_SYS_CMD(cmd, q, cd, ret)               \
    {                                                       \
    case REQ_CMD_CACHE_GET:                                 \
        ret = sys_cmd_cache_get(q, cd, false);      \
        break;                                              \
    case REQ_CMD_CACHE_SET:                                 \
        ret = sys_cmd_cache_set(q, cd, false);      \
        break;                                              \
    case REQ_CMD_CACHE_DEL:                                 \
        ret = sys_cmd_cache_del(q, cd, false);      \
        break;                                              \
    case REQ_CMD_CACHE_EMPTY:                               \
        ret = sys_cmd_cache_empty(q, &cd, false);   \
        break;                                              \
    }
        
int sys_cmd_cache_get(struct queue_entry *q, struct cache *cd, bool reply);
int sys_cmd_cache_set(struct queue_entry *q, struct cache *cd, bool reply);
int sys_cmd_cache_del(struct queue_entry *q, struct cache *cd, bool reply);
int sys_cmd_cache_empty(struct queue_entry *q, struct cache **cd, bool reply);

#endif  /* __SYS_CMD_H__ */
