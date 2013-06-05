
#if ENABLE_TCP

#include <sys/types.h>        /* socket defines */
#include <sys/socket.h>        /* socket functions */
#include <stdlib.h>        /* malloc() */
#include <stdint.h>        /* uint32_t and friends */
#include <arpa/inet.h>        /* htonls() and friends */
#include <string.h>        /* memcpy() */
#include <unistd.h>        /* close() */
#include <errno.h>

#include <netinet/tcp.h>    /* TCP stuff */
#include <netdb.h>        /* gethostbyname() */
#include <fcntl.h>

#include "mevent.h"
#include "net-const.h"
#include "internal.h"
#include "tcp.h"


/* Used internally to really add the server once we have an IP address. */
static int add_tcp_server_addr(mevent_t *evt, in_addr_t *inetaddr, int port,
                               const char *nblock, struct timeval *tv)
{
    int rv, fd;
    struct mevent_srv *newsrv, *newarray;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return 0;

    if (nblock && !strcmp(nblock, "yes")) {
        int x = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, x | O_NONBLOCK);
    } else {
        if (tv->tv_sec != 0 || tv->tv_usec != 0) {
            setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)tv, sizeof(*tv));
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)tv, sizeof(*tv));
        }
    }
    
    newarray = realloc(evt->servers,
               sizeof(struct mevent_srv) * (evt->nservers + 1));
    if (newarray == NULL) {
        close(fd);
        return 0;
    }

    evt->servers = newarray;
    evt->nservers++;

    if (port < 0)
        port = TCP_SERVER_PORT;

    newsrv = &(evt->servers[evt->nservers - 1]);

    newsrv->fd = fd;
    newsrv->info.in.srvsa.sin_family = AF_INET;
    newsrv->info.in.srvsa.sin_port = htons(port);
    newsrv->info.in.srvsa.sin_addr.s_addr = *inetaddr;
    newsrv->nblock = strdup(nblock);
    newsrv->tv.tv_sec = tv->tv_sec;
    newsrv->tv.tv_usec = tv->tv_usec;

    rv = connect(fd, (struct sockaddr *) &(newsrv->info.in.srvsa),
             sizeof(newsrv->info.in.srvsa));
    if (rv < 0)
        goto error_exit;

    /* Disable Nagle algorithm because we often send small packets. Huge
     * gain in performance. */
    rv = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &rv, sizeof(rv)) < 0 )
        goto error_exit;

    newsrv->type = TCP_CONN;

    /* keep the list sorted by port, so we can do a reliable selection */
    qsort(evt->servers, evt->nservers, sizeof(struct mevent_srv),
          compare_servers);

    return 1;

 error_exit:
    close(fd);
    newarray = realloc(evt->servers,
               sizeof(struct mevent_srv) * (evt->nservers - 1));
    if (newarray == NULL) {
        evt->servers = NULL;
        evt->nservers = 0;
        return 0;
    }

    evt->servers = newarray;
    evt->nservers -= 1;

    return 0;
}

static int tcp_server_reconnect(struct mevent_srv *srv)
{
    int rv, fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 0;

    if (srv->nblock && !strcmp(srv->nblock, "yes")) {
        int x = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, x | O_NONBLOCK);
    } else {
        if (srv->tv.tv_sec != 0 || srv->tv.tv_usec != 0) {
            setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&(srv->tv), sizeof(srv->tv));
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&(srv->tv), sizeof(srv->tv));
        }
    }
    
    rv = connect(fd, (struct sockaddr *) &(srv->info.in.srvsa),
                 sizeof(srv->info.in.srvsa));
    if (rv < 0) goto error_exit;

    /*
     * Disable Nagle algorithm because we often send small packets.
     * Huge gain in performance.
     */
    rv = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &rv, sizeof(rv)) < 0 ) goto error_exit;

    srv->fd = fd;

    return 1;

error_exit:
    close(fd);

    return 0;
}

/* Same as mevent_add_tipc_server() but for TCP connections. */
int mevent_add_tcp_server(mevent_t *evt, const char *addr, int port,
                          const char *nblock, void *tv)
{
    int rv;
    struct hostent *he;
    struct in_addr ia;

    /* We try to resolve and then pass it to add_tcp_server_addr(). */
    rv = inet_pton(AF_INET, addr, &ia);
    if (rv <= 0) {
        he = gethostbyname(addr);
        if (he == NULL)
            return 0;

        ia.s_addr = *( (in_addr_t *) (he->h_addr_list[0]) );
    }

    return add_tcp_server_addr(evt, &(ia.s_addr), port, nblock, (struct timeval*)tv);
}

int tcp_srv_send(struct mevent_srv *srv, unsigned char *buf, size_t bsize)
{
    ssize_t rv;
    uint32_t len;

    len = htonl(bsize);
    memcpy(buf, (const void *) &len, 4);

    if (srv->fd <= 0) {
        if (!tcp_server_reconnect(srv)) return 0;
    }

    rv = ssend(srv->fd, buf, bsize, MSG_NOSIGNAL);
    if (rv != bsize) {
        if (rv < 0 && errno == EPIPE) {
            close(srv->fd);
            srv->fd = -1;
            /*
            if (!tcp_server_reconnect(srv)) return 0;
            rv = ssend(srv->fd, buf, bsize, MSG_NOSIGNAL);
            if (rv == bsize) return 1;
            */
        }
        return 0;
    }
    return 1;
}

static ssize_t recv_msg(int fd, unsigned char *buf, size_t bsize)
{
    ssize_t rv, t;
    uint32_t msgsize;

    rv = recv(fd, buf, bsize, MSG_NOSIGNAL);
    if (rv <= 0)
        return rv;

    if (rv < 4) {
        t = srecv(fd, buf + rv, 4 - rv, MSG_NOSIGNAL);
        if (t <= 0) {
            return t;
        }

        rv = rv + t;
    }

    msgsize = * ((uint32_t *) buf);
    msgsize = ntohl(msgsize);

    if (msgsize > bsize)
        return -1;

    if (rv < msgsize) {
        t = srecv(fd, buf + rv, msgsize - rv, MSG_NOSIGNAL);
        if (t <= 0) {
            return t;
        }

        rv = rv + t;
    }

    return rv;
}


/* Used internally to get and parse replies from the server. */
uint32_t tcp_get_rep(struct mevent_srv *srv,
             unsigned char *buf, size_t bsize,
             unsigned char **payload, size_t *psize)
{
    ssize_t rv;
    uint32_t id, reply;

rerecv:
    rv = recv_msg(srv->fd, buf, bsize);
    if (rv <= 0) {
        if (errno != EAGAIN) {
            /*
             * orderly shutdown or error
             */
            close(srv->fd);
            srv->fd = -1;
        }
        
        return -1;
    }

    id = * ((uint32_t *) buf + 1);
    id = ntohl(id);
    reply = * ((uint32_t *) buf + 2);
    reply = ntohl(reply);

    if (id < g_reqid) goto rerecv;

    if (payload != NULL) {
        *payload = buf + 4 + 4 + 4;
        *psize = rv - 4 - 4 - 4;
    }
    return reply;
}

#else
/* Stubs to use when TCP is not enabled. */

#include <stdint.h>
#include "mevent.h"
#include "tcp.h"

int mevent_add_tcp_server(mevent_t *evt, const char *addr, int port)
{
    return 0;
}

int tcp_srv_send(struct mevent_srv *srv, unsigned char *buf, size_t bsize)
{
    return 0;
}

uint32_t tcp_get_rep(struct mevent_srv *srv,
             unsigned char *buf, size_t bsize,
             unsigned char **payload, size_t *psize)
{
    return -1;
}

#endif /* ENABLE_TCP */

