#ifndef _PARSE_H
#define _PARSE_H

#include "apev.h"

#ifdef DEBUG_MSG
#include "mheads.h"
#define MSG_DUMP(pre, p, psize)                                         \
    do {                                                                \
        unsigned char zstra[MAX_PACKET_LEN*2+1];                        \
        mstr_bin2char((unsigned char*)p, (unsigned int)psize, zstra);   \
        mtc_dbg("%s%s", pre, zstra);                                    \
    } while (0)
#else
#define MSG_DUMP(pre, p, psize)
#endif

NEOERR* parse_regist_v(struct event_entry *e);
NEOERR* parse_message(struct req_info *req, const unsigned char *buf, size_t len);

#endif
