#include "apev.h"

static void rep_send_error(const struct req_info *req, const unsigned int code)
{
    int r, c;
    unsigned char minibuf[3 * 4];

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
        /* TODO log... */
        //errlog("rep_send_error() failed");
    }
}

static void udp_reply_mini(const struct req_info *req, uint32_t reply);
static int rep_send(const struct req_info *req, const unsigned char *buf,
        const size_t size)
{
    int rv;

    MSG_DUMP("send: ",  buf, size);
    
    rv = sendto(req->fd, buf, size, 0, req->clisa, req->clilen);
    if (rv < 0) {
        //rep_send_error(req, ERR_SEND);
        udp_reply_mini(req, REP_ERR_SEND);
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

int udps_init(char *ip, int port)
{
    int fd, rv;
    struct sockaddr_in srvsa;
    struct in_addr ia;
    
    if (inet_pton(AF_INET, ip, &ia) <= 0) return -1;
    srvsa.sin_family = AF_INET;
    srvsa.sin_addr.s_addr = ia.s_addr;
    srvsa.sin_port = htons(port);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &rv, sizeof(rv)) < 0) {
        close(fd);
        return -1;
    }

    if (bind(fd, (struct sockaddr*)&srvsa, sizeof(srvsa)) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

#define SBSIZE (68 * 1024)
static unsigned char static_buf[SBSIZE];

NEOERR* udps_recv(int fd, short event, void *arg)
{
    struct sockaddr_in clisa;
    socklen_t clilen;
    struct req_info req;
    int rv;

    clilen = sizeof(clisa);

    rv = recvfrom(fd, static_buf, SBSIZE, 0, (struct sockaddr*)&clisa, &clilen);
    /* TODO stat */
    if (rv < 8) return nerr_raise(NERR_ASSERT, "recvfrom %d bytes from %d", rv, fd);

    req.fd = fd;
    req.type = REQTYPE_UDP;
    req.clisa = (struct sockaddr*)&clisa;
    req.clilen = clilen;
    req.reply_mini = udp_reply_mini;
    req.reply_err = udp_reply_err;
    req.reply_long = udp_reply_long;

    return nerr_pass(parse_message(&req, static_buf, rv));
}

void udps_close(int fd)
{
    close(fd);
}
