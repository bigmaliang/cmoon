#include <stdlib.h>        /* malloc() */
#include <stdint.h>        /* uint32_t and friends */
#include <stdbool.h>        /* bool */
#include <string.h>        /* memcpy() */
#include <arpa/inet.h>        /* htonl() and friends */

#include "net-const.h"
#include "packet.h"
#include "queue.h"

int reply_trigger(struct queue_entry *q, uint32_t reply)
{
    if (q == NULL) return 0;

    if (q->hdfsnd == NULL || hdf_obj_child(q->hdfsnd) == NULL) {
        q->req->reply_mini(q->req, reply);
        return 1;
    }
    
    unsigned char *buf = calloc(1, MAX_PACKET_LEN);
    if (buf == NULL) {
        q->req->reply_mini(q->req, REP_ERR_MEM);
        return 0;
    }

    size_t vsize;
    vsize = pack_hdf(q->hdfsnd, buf, MAX_PACKET_LEN);
    if (vsize == 0) goto error;
 
    q->req->reply_long(q->req, reply, buf, vsize);

    free(buf);
    return 1;
    
 error:
    q->req->reply_mini(q->req, REP_ERR_PACK);
    free(buf);
    return 0;
}
