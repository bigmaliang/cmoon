
#include <sys/types.h>        /* socket defines */
#include <sys/socket.h>        /* socket functions */
#include <stdlib.h>        /* malloc() */
#include <stdint.h>        /* uint32_t and friends */
#include <arpa/inet.h>        /* htonls() and friends */
#include <netinet/in.h>        /* INET stuff */
#include <netinet/udp.h>    /* UDP stuff */
#include <string.h>        /* memcpy() */
#include <unistd.h>        /* close() */

#include "udp.h"
#include "common.h"
#include "net-const.h"
#include "req.h"
#include "parse.h"
#include "log.h"


/*
 * Miscelaneous helper functions
 */

static void rep_send_error(const struct req_info *req, const unsigned int code)
{
    int r, c;
    unsigned char minibuf[3 * 4];

    if (settings.passive)
        return;

    /* Network format: ID (4), REP_ERR (4), error code (4) */
    r = htonl(REP_ERR);
    c = htonl(code);
    memcpy(minibuf, &(req->id), 4);
    memcpy(minibuf + 4, &r, 4);
    memcpy(minibuf + 8, &c, 4);

    MSG_DUMP("send: ",  minibuf, 3 * 4);
    
    /* If this send fails, there's nothing to be done */
    r = sendto(req->fd, minibuf, 3 * 4, 0, req->clisa, req->clilen);

    if (r < 0) {
        mtc_err("rep_send_error() failed");
    }
}

static void udp_reply_mini(const struct req_info *req, uint32_t reply);
static int rep_send(const struct req_info *req, const unsigned char *buf,
        const size_t size)
{
    int rv;

    if (settings.passive)
        return 1;

    MSG_DUMP("send: ",  buf, size);
    
    rv = sendto(req->fd, buf, size, 0, req->clisa, req->clilen);
    if (rv < 0) {
        rep_send_error(req, REP_ERR_SEND);
        /* udp_reply_mini will call rep_send(), cause core dume */
        //udp_reply_mini(req, REP_ERR_SEND);
        return 0;
    }
    return 1;
}


/* Send small replies, consisting in only a value. */
static void udp_reply_mini(const struct req_info *req, uint32_t reply)
{
    /* We use a mini buffer to speedup the small replies, to avoid the
     * malloc() overhead. */
    unsigned char minibuf[8];

    if (settings.passive)
        return;

    reply = htonl(reply);
    memcpy(minibuf, &(req->id), 4);
    memcpy(minibuf + 4, &reply, 4);
    rep_send(req, minibuf, 8);
    return;
}


/* The udp_reply_* functions are used by the db code to send the network
 * replies. */

static void udp_reply_err(const struct req_info *req, uint32_t reply)
{
    rep_send_error(req, reply);
}

static void udp_reply_long(const struct req_info *req, uint32_t reply,
            unsigned char *val, size_t vsize)
{
    if (val == NULL) {
        /* miss */
        udp_reply_mini(req, reply);
    } else {
        unsigned char *buf;
        size_t bsize;
        uint32_t t;

        reply = htonl(reply);

        /* The reply length is:
         * 4        id
         * 4        reply code
         * 4        vsize
         * vsize    val
         */
        bsize = 4 + 4 + 4 + vsize;
        buf = malloc(bsize);

        t = htonl(vsize);

        memcpy(buf, &(req->id), 4);
        memcpy(buf + 4, &reply, 4);
        memcpy(buf + 8, &t, 4);
        memcpy(buf + 12, val, vsize);

        rep_send(req, buf, bsize);
        free(buf);
    }
    return;

}


/*
 * Main functions for receiving and parsing
 */

int udp_init(void)
{
    int fd, rv;
    struct sockaddr_in srvsa;
    struct in_addr ia;

    rv = inet_pton(AF_INET, settings.udp_addr, &ia);
    if (rv <= 0)
        return -1;

    srvsa.sin_family = AF_INET;
    srvsa.sin_addr.s_addr = ia.s_addr;
    srvsa.sin_port = htons(settings.udp_port);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return -1;

    rv = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &rv, sizeof(rv)) < 0 ) {
        close(fd);
        return -1;
    }

    rv = bind(fd, (struct sockaddr *) &srvsa, sizeof(srvsa));
    if (rv < 0) {
        close(fd);
        return -1;
    }

    return fd;
}


void udp_close(int fd)
{
    close(fd);
}


/* Static common buffer to avoid unnecessary allocations. See the comments on
 * this same variable in tipc.c. */
#define SBSIZE (68 * 1024)
static unsigned char static_buf[SBSIZE];

/* Called by libevent for each receive event */
void udp_recv(int fd, short event, void *arg)
{
    int rv;
    struct req_info req;
    struct sockaddr_in clisa;
    socklen_t clilen;

    clilen = sizeof(clisa);

    rv = recvfrom(fd, static_buf, SBSIZE, 0, (struct sockaddr *) &clisa,
            &clilen);
    if (rv < 0) {
        goto exit;
    }

    if (rv < 8) {
        stats.net_broken_req++;
        goto exit;
    }

    stats.msg_udp++;

    req.fd = fd;
    req.type = REQTYPE_UDP;
    req.clisa = (struct sockaddr *) &clisa;
    req.clilen = clilen;
    req.reply_mini = udp_reply_mini;
    req.reply_err = udp_reply_err;
    req.reply_long = udp_reply_long;

    /* parse the message */
    parse_message(&req, static_buf, rv);

exit:
    return;
}


