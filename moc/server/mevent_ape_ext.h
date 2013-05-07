#ifndef __MEVENT_APEEXT_H__
#define __MEVENT_APEEXT_H__

#include <stdbool.h>

enum {
    REQ_CMD_USERON  = 1001,
    REQ_CMD_USEROFF,
    REQ_CMD_MSGSND  = 2001,
    REQ_CMD_MSGBRD,
    REQ_CMD_CHAN_MISS,
    REQ_CMD_CHAN_ATTEND,
    REQ_CMD_CHAN_INFO,
    REQ_CMD_CONNECT = 3001,        /* called before CMD_CONNECT, for load balance. */
    REQ_CMD_HB      = 4001,        /* x send to v, keep heart beat */
    REQ_CMD_STATE = 9001
};

enum {
    REP_ERR_ALLDIE = 25
};

void ext_snake_sort();

#endif    /* __MEVENT_APEEXT_H__ */
