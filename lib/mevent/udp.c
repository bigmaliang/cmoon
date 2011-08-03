
#if ENABLE_UDP

#include <sys/types.h>        /* socket defines */
#include <sys/socket.h>        /* socket functions */
#include <stdlib.h>        /* malloc() */
#include <stdint.h>        /* uint32_t and friends */
#include <arpa/inet.h>        /* htonls() and friends */
#include <string.h>        /* memcpy() */
#include <unistd.h>        /* close() */
#include <time.h>
#include <sys/time.h>

#include <netinet/udp.h>    /* UDP stuff */
#include <netdb.h>        /* gethostbyname() */
#include <fcntl.h>

#include "mevent.h"
#include "net-const.h"
#include "internal.h"
#include "udp.h"


/* Used internally to really add the server once we have an IP address. */
static int add_udp_server_addr(mevent_t *evt, in_addr_t *inetaddr, int port,
                               const char *nblock, struct timeval *tv)
{
    int fd;
    struct mevent_srv *newsrv, *newarray;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
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
        port = UDP_SERVER_PORT;

    newsrv = &(evt->servers[evt->nservers - 1]);

    newsrv->fd = fd;
    newsrv->info.in.srvsa.sin_family = AF_INET;
    newsrv->info.in.srvsa.sin_port = htons(port);
    newsrv->info.in.srvsa.sin_addr.s_addr = *inetaddr;
    newsrv->info.in.srvlen = sizeof(struct sockaddr_in);

    newsrv->type = UDP_CONN;

    /* keep the list sorted by port, so we can do a reliable selection */
    qsort(evt->servers, evt->nservers, sizeof(struct mevent_srv),
          compare_servers);

    return 1;
}

/* Same as mevent_add_tcp_server() but for UDP. */
int mevent_add_udp_server(mevent_t *evt, const char *addr, int port,
                          const char *nblock, void *tv)
{
    int rv;
    struct hostent *he;
    struct in_addr ia;

    /* We try to resolve and then pass it to add_udp_server_addr(). */
    rv = inet_pton(AF_INET, addr, &ia);
    if (rv <= 0) {
        he = gethostbyname(addr);
        if (he == NULL)
            return 0;

        ia.s_addr = *( (in_addr_t *) (he->h_addr_list[0]) );
    }

    return add_udp_server_addr(evt, &(ia.s_addr), port, nblock, (struct timeval*)tv);
}

int udp_srv_send(struct mevent_srv *srv, unsigned char *buf, size_t bsize)
{
    ssize_t rv;
    rv = sendto(srv->fd, buf, bsize, 0,
                (struct sockaddr *) &(srv->info.in.srvsa),
                srv->info.in.srvlen);
    if (rv <= 0)
        return 0;
    return 1;
}

/* Used internally to get and parse replies from the server. */
uint32_t udp_get_rep(struct mevent_srv *srv,
                     unsigned char *buf, size_t bsize,
                     unsigned char **payload, size_t *psize)
{
    ssize_t rv;
    uint32_t id, reply;

rerecv:
    rv = recv(srv->fd, buf, bsize, 0);
    if (rv < 4 + 4) {
        return -1;
    }

    id = * (uint32_t *) buf;
    id = ntohl(id);
    reply = * ((uint32_t *) buf + 1);
    reply = ntohl(reply);

    if (id < g_reqid) goto rerecv;
    //if (id != ID_CODE) {
    //    return -1;
    //}

    if (payload != NULL) {
        *payload = buf + 4 + 4;
        *psize = rv - 4 - 4;
    }
    return reply;
}

#else
/* Stubs to use when UDP is not enabled. */

#include <stdint.h>
#include "mevent.h"
#include "udp.h"

int mevent_add_udp_server(mevent_t *evt, const char *addr, int port)
{
    return 0;
}

int udp_srv_send(struct mevent_srv *srv, unsigned char *buf, size_t bsize)
{
    return 0;
}

uint32_t udp_get_rep(struct mevent_srv *srv,
                     unsigned char *buf, size_t bsize,
                     unsigned char **payload, size_t *psize)
{
    return -1;
}

#endif /* ENABLE_UDP */

