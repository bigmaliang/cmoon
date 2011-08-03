#ifndef _REQ_H
#define _REQ_H

#include <stdint.h>        /* uint32_t */
#include <sys/types.h>        /* size_t */
#include <sys/socket.h>        /* socklen_t */

#include "ClearSilver.h"


/* req_info types, according to the protocol */
#define REQTYPE_TIPC 1
#define REQTYPE_TCP 2
#define REQTYPE_UDP 3
#define REQTYPE_SCTP 4


struct req_info {
    /* network information */
    int fd;
    int type;

    struct sockaddr *clisa;
    socklen_t clilen;

    /* operation information */
    uint32_t id;
    uint16_t cmd;
    uint16_t flags;
    const unsigned char *payload;
    size_t psize;

    /* operations */
    /* reply_err is depracated */
    void (*reply_mini)(const struct req_info *req, uint32_t reply);
    void (*reply_err)(const struct req_info *req, uint32_t reply);
    void (*reply_long)(const struct req_info *req, uint32_t reply,
            unsigned char *val, size_t vsize);
};


struct queue_entry {
    unsigned char *ename;
    size_t esize;
    
    uint32_t operation;
    struct req_info *req;

    HDF *hdfrcv;
    HDF *hdfsnd;
};

struct event_entry {
    /*
     * different by plugin, init in init_driver()
     */
    char *name;
    NEOERR* (*start_driver)(void);
    NEOERR* (*process_driver)(struct event_entry *e, struct queue_entry *q);
    void (*stop_driver)(struct event_entry *e);

    struct event_entry *next;
};

#endif

