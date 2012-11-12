#include <sys/types.h>    /* socket defines */
#include <sys/socket.h>    /* socket functions */
#include <stdlib.h>        /* malloc() */
#include <stdint.h>        /* uint32_t and friends */
#include <stdbool.h>    /* bool, true, false */
#include <arpa/inet.h>    /* htonls() and friends */
#include <string.h>        /* memcpy() */
#include <unistd.h>        /* close() */

#include "mevent.h"
#include "net-const.h"
#include "tipc.h"
#include "tcp.h"
#include "udp.h"
#include "sctp.h"
#include "internal.h"
#include "netutils.h"
#include "packet.h"

#define CONFIG_FILE        "/etc/mevent/client.hdf"

static bool loaded = false;
static HDF *g_cfg;
unsigned int g_reqid = 0;

static int load_config()
{
    NEOERR *err;

    if (loaded) return 1;
    
    if (g_cfg != NULL) {
        hdf_destroy(&g_cfg);
    }
    err = hdf_init(&g_cfg);
    if (err != STATUS_OK) {
        nerr_ignore(&err);
        return 0;
    }

    err = hdf_read_file(g_cfg, CONFIG_FILE);
    if (err != STATUS_OK) {
        nerr_ignore(&err);
        return 0;
    }

    loaded = true;
    return 1;
}

/* Compares two servers by their connection identifiers. It is used internally
 * to keep the server array sorted with qsort(). */
int compare_servers(const void *s1, const void *s2)
{
    struct mevent_srv *srv1 = (struct mevent_srv *) s1;
    struct mevent_srv *srv2 = (struct mevent_srv *) s2;

    if (srv1->type != srv2->type) {
        if (srv1->type < srv2->type)
            return -1;
        else
            return 1;
    }

#if ENABLE_TIPC
    if (srv1->type == TIPC_CONN) {
        if (srv1->info.tipc.port < srv2->info.tipc.port)
            return -1;
        else if (srv1->info.tipc.port == srv2->info.tipc.port)
            return 0;
        else
            return 1;
    }
#endif
#if ENABLE_TCP || ENABLE_UDP || ENABLE_SCTP
    if (srv1->type == TCP_CONN
        || srv1->type == UDP_CONN
        || srv1->type == SCTP_CONN) {
        in_addr_t a1, a2;
        a1 = srv1->info.in.srvsa.sin_addr.s_addr;
        a2 = srv2->info.in.srvsa.sin_addr.s_addr;

        if (a1 < a2) {
            return -1;
        } else if (a1 == a2) {
            in_port_t p1, p2;
            p1 = srv1->info.in.srvsa.sin_port;
            p2 = srv2->info.in.srvsa.sin_port;

            if (p1 < p2)
                return -1;
            else if (p1 == p2)
                return 0;
            else
                return 1;
        } else {
            return 1;
        }
    }
#endif

    /* We should never get here */
    return 0;
}


/* Like recv(), but either fails, or returns a complete read; if we return
 * less than count is because EOF was reached. */
ssize_t srecv(int fd, unsigned char *buf, size_t count, int flags)
{
    ssize_t rv, c;

    c = 0;

    while (c < count) {
        rv = recv(fd, buf + c, count - c, flags);

        if (rv == count)
            return count;
        else if (rv < 0)
            return rv;
        else if (rv == 0)
            return c;

        c += rv;
    }
    return count;
}

/* Like srecv(), but for send(). */
ssize_t ssend(int fd, const unsigned char *buf, size_t count, int flags)
{
    ssize_t rv, c;

    c = 0;

    while (c < count) {
        rv = send(fd, buf + c, count - c, flags);

        if (rv == count)
            return count;
        else if (rv < 0)
            return rv;
        else if (rv == 0)
            return c;

        c += rv;
    }
    return count;
}

/*
 * mevent system error
 */
int REP_ERR = 0;                /* 14 */
int REP_ERR_VER = 0;            /* 15 */
int REP_ERR_SEND = 0;           /* 16 */
int REP_ERR_BROKEN = 0;         /* 17 */
int REP_ERR_UNKREQ = 0;         /* 18 */
int REP_ERR_MEM = 0;            /* 19 */
int REP_ERR_DB = 0;             /* 20 */
int REP_ERR_BUSY = 0;           /* 21 */
int REP_ERR_PACK = 0;           /* 22 */
int REP_ERR_BADPARAM = 0;       /* 23 */
int REP_ERR_CACHE_MISS = 0;     /* 24 */

static int merr_inited = 0;

MeventLog mevent_log = NULL;

NEOERR *merr_init(MeventLog logf)
{
    NEOERR *err;

    if (logf) mevent_log = logf;
    
    if (merr_inited) return STATUS_OK;

    err = nerr_register(&REP_ERR, "后台处理失败");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register(&REP_ERR_VER, "通信协议版本不对");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register(&REP_ERR_SEND, "后台处理发送失败");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register(&REP_ERR_BROKEN, "后台网络包丢失");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register(&REP_ERR_UNKREQ, "事件后台无效");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register(&REP_ERR_MEM, "后台内存错误");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register(&REP_ERR_DB, "后台数据库错误");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register(&REP_ERR_BUSY, "后台繁忙");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register(&REP_ERR_PACK, "后台打包失败");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register(&REP_ERR_BADPARAM, "后台参数错误");
    if (err != STATUS_OK) return nerr_pass(err);
    err = nerr_register(&REP_ERR_CACHE_MISS, "后台缓存获取失败");
    if (err != STATUS_OK) return nerr_pass(err);

    merr_inited = 1;
    
    return STATUS_OK;
}

/* Creates a mevent_t. */
mevent_t* mevent_init(char *ename)
{
    mevent_t *evt;
    NEOERR *err;

    if (!ename) return NULL;

    evt = malloc(sizeof(mevent_t));
    if (evt == NULL) {
        return NULL;
    }

    evt->cmd = REQ_CMD_NONE;
    evt->flags = FLAGS_NONE;
    evt->errcode = REP_OK;
    
    char s[64];
    neo_rand_string(s, 60);
    evt->key = strdup(s);

    evt->ename = strdup(ename);
    
    evt->servers = NULL;
    evt->nservers = 0;
    err = hdf_init(&evt->hdfsnd);
    if (err != STATUS_OK) {
        free(evt);
        return NULL;
    }
    evt->packed = 0;
    evt->payload = calloc(1, MAX_PACKET_LEN);
    if (evt->payload == NULL) {
        free(evt);
        return NULL;
    }
    evt->rcvbuf = calloc(1, MAX_PACKET_LEN);
    if (evt->rcvbuf == NULL) {
        free(evt);
        return NULL;
    }
    evt->psize = 0;
    err = hdf_init(&evt->hdfrcv);
    if (err != STATUS_OK) {
        hdf_destroy(&evt->hdfsnd);
        free(evt);
        return NULL;
    }

    return evt;
}

mevent_t *mevent_init_plugin(char *ename)
{
    char *type, *ip, *nblock;
    int port;
    struct timeval tv;
    
    if (!ename)    return NULL;
    
    if (!loaded || g_cfg == NULL) {
        if (load_config() != 1) {
            return NULL;
        }
    }

    HDF *node = hdf_get_obj(g_cfg, ename);
    if (!node) return NULL;
    
    mevent_t *evt = mevent_init(ename);
    if (!evt) return NULL;

    node = hdf_obj_child(node);
    while (node != NULL) {
        type = hdf_get_value(node, "type", "unknown");
        ip = hdf_get_value(node, "ip", "127.0.0.1") ;
        port = hdf_get_int_value(node, "port", 26010);
        nblock = hdf_get_value(node, "non_block", NULL);
        tv.tv_sec = hdf_get_int_value(node, "net_timeout_s", 0);
        tv.tv_usec = hdf_get_int_value(node, "net_timeout_u", 0);
            
        if (!strcmp(type, "tcp")) {
            if (!mevent_add_tcp_server(evt, ip, port, nblock, &tv)) return NULL;
        } else if (!strcmp(type, "udp")) {
            if (!mevent_add_udp_server(evt, ip, port, nblock, &tv)) return NULL;
        } else if (!strcmp(type, "tipc")) {
            if (!mevent_add_tipc_server(evt, port)) return NULL;
        } else if (!strcmp(type, "sctp")) {
            if (!mevent_add_sctp_server(evt, ip, port)) return NULL;
        } else {
            continue;
        }

        node = hdf_obj_next(node);
    }

    if (evt->nservers <= 0) {
        mevent_free(evt);
        return NULL;
    }

    return evt;
}

/* Frees a mevent_t structure created with mevent_init(). */
void mevent_free(void *p)
{
    mevent_t *evt = p;
    if (evt == NULL) return;
    
    if (evt->servers != NULL) {
        int i;
        for (i = 0; i < evt->nservers; i++)
            close(evt->servers[i].fd);
        free(evt->servers);
    }
    if (evt->ename != NULL)
        free(evt->ename);
    if (evt->key != NULL)
        free(evt->key);
    hdf_destroy(&evt->hdfsnd);
    if (evt->payload != NULL)
        free(evt->payload);
    if (evt->rcvbuf != NULL)
        free(evt->rcvbuf);
    hdf_destroy(&evt->hdfrcv);
    free(evt);
}

/* Sends a buffer to the given server. */
static int srv_send(struct mevent_srv *srv,
                    unsigned char *buf, size_t bsize)
{
    if (srv == NULL)
        return 0;

    switch (srv->type) {
    case TIPC_CONN:
        return tipc_srv_send(srv, buf, bsize);
    case TCP_CONN:
        return tcp_srv_send(srv, buf, bsize);
    case UDP_CONN:
        return udp_srv_send(srv, buf, bsize);
    case SCTP_CONN:
        return sctp_srv_send(srv, buf, bsize);
    default:
        return 0;
    }
}

/* Gets a reply from the given server. */
static uint32_t get_rep(struct mevent_srv *srv,
                        unsigned char *buf, size_t bsize,
                        unsigned char **payload, size_t *psize)
{
    if (srv == NULL)
        return -1;

    switch (srv->type) {
    case TIPC_CONN:
        return tipc_get_rep(srv, buf, bsize, payload, psize);
    case TCP_CONN:
        return tcp_get_rep(srv, buf, bsize, payload, psize);
    case UDP_CONN:
        return udp_get_rep(srv, buf, bsize, payload, psize);
    case SCTP_CONN:
        return sctp_get_rep(srv, buf, bsize, payload, psize);
    default:
        return 0;
    }
}

/* When a packet arrives, the message it contains begins on a
 * protocol-dependant offset. This functions returns the offset to use when
 * sending/receiving messages for the given server. */
unsigned int srv_get_msg_offset(struct mevent_srv *srv)
{
    if (srv == NULL)
        return 0;

    switch (srv->type) {
    case TIPC_CONN:
        return TIPC_MSG_OFFSET;
    case TCP_CONN:
        return TCP_MSG_OFFSET;
    case UDP_CONN:
        return UDP_MSG_OFFSET;
    case SCTP_CONN:
        return SCTP_MSG_OFFSET;
    default:
        return 0;
    }
}


/* Hash function used internally by select_srv(). See RFC 1071. */
static uint32_t checksum(const unsigned char *buf, size_t bsize)
{
    uint32_t sum = 0;

    while (bsize > 1)  {
        sum += * (uint16_t *) buf++;
        bsize -= 2;
    }

    if (bsize > 0)
        sum += * (uint8_t *) buf;

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

/* Selects which server to use for the given key. */
struct mevent_srv *select_srv(mevent_t *evt,
                              const char *key, size_t ksize)
{
    uint32_t n;
    
    if (evt->nservers == 0)
        return NULL;
    
    n = checksum((const unsigned char*)key, ksize) % evt->nservers;
    return &(evt->servers[n]);
}


int mevent_trigger(mevent_t *evt, char *key,
                   unsigned short cmd, unsigned short flags)
{
    size_t t, ksize, vsize;
    struct mevent_srv *srv;
    unsigned int moff;
    unsigned char *p;
    uint32_t rv = REP_OK;
    
    if (!evt) return REP_ERR;

    if (!key) key = evt->key;
    evt->cmd = cmd;
    evt->flags = flags;
    ksize = strlen(evt->ename);

    srv = select_srv(evt, key, strlen(key));
    if (!srv) {
        evt->errcode = REP_ERR;
        return REP_ERR;
    }
    
    if (g_reqid++ > 0x0FFFFFFC) {
        g_reqid = 1;
    }
    
    moff = srv_get_msg_offset(srv);
    p = evt->payload + moff;
    //* (uint32_t *) p = htonl( (PROTO_VER << 28) | ID_CODE );
    * (uint32_t *) p = htonl( (PROTO_VER << 28) | g_reqid );
    * ((uint16_t *) p + 2) = htons(cmd);
    * ((uint16_t *) p + 3) = htons(flags);
    * ((uint32_t *) p + 2) = htonl(ksize);
    memcpy(p+12, evt->ename, ksize);

    evt->psize = moff + 12 +ksize;
    
    /*
     * don't escape the hdf because some body need set ' in param 
     */
    vsize = pack_hdf(evt->hdfsnd, evt->payload + evt->psize, MAX_PACKET_LEN);
    evt->psize += vsize;

    if (evt->psize < 17) {
        * (uint32_t *) (evt->payload+evt->psize) = htonl(DATA_TYPE_EOF);
        evt->psize += sizeof(uint32_t);
    }
    
    t = srv_send(srv, evt->payload, evt->psize);
    if (t <= 0) {
        evt->errcode = REP_ERR_SEND;
        return REP_ERR_SEND;
    }

    hdf_destroy(&evt->hdfsnd);
    hdf_init(&evt->hdfsnd);
    hdf_destroy(&evt->hdfrcv);
    hdf_init(&evt->hdfrcv);
    
    if (flags & FLAGS_SYNC) {
        vsize = 0;
        rv = get_rep(srv, evt->rcvbuf, MAX_PACKET_LEN, &p, &vsize);
        if (rv == -1) rv = REP_ERR;
        evt->errcode = rv;

        if (vsize > 8) {
            /* reply_long add a vsize parameter */
            unpack_hdf(p+4, vsize-4, &evt->hdfrcv);
        }
    }

    return rv;
}
