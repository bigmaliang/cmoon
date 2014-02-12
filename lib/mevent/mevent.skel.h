
/* Header for the libmevent library. */

#ifndef _MEVENT_H
#define _MEVENT_H

#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif

#include "net-const.h"

#include "ClearSilver.h"

/* Defined to 0 or 1 at libmevent build time according the build configuration,
 * not to be used externally. */
#define _ENABLE_TIPC ++CONFIG_ENABLE_TIPC++
#define _ENABLE_TCP ++CONFIG_ENABLE_TCP++
#define _ENABLE_UDP ++CONFIG_ENABLE_UDP++
#define _ENABLE_SCTP ++CONFIG_ENABLE_SCTP++


#include <sys/types.h>        /* socket defines */
#include <sys/socket.h>        /* socklen_t */

#if _ENABLE_TIPC
#include <linux/tipc.h>        /* struct sockaddr_tipc */
#endif

#if (_ENABLE_TCP || _ENABLE_UDP || _ENABLE_SCTP)
#include <netinet/in.h>        /* struct sockaddr_in */
#endif

struct mevent_srv {
    int fd;
    int type;
    char *nblock;
    struct timeval tv;
    union {

#if _ENABLE_TIPC
        struct {
            unsigned int port;
            struct sockaddr_tipc srvsa;
            socklen_t srvlen;
        } tipc;
#endif

#if (_ENABLE_TCP || _ENABLE_UDP || _ENABLE_SCTP)
        struct {
            struct sockaddr_in srvsa;
            socklen_t srvlen;
        } in;
#endif

    } info;
};

typedef struct mevent_t {
    unsigned int nservers;
    struct mevent_srv *servers;
    int cmd;
    int flags;
    int errcode;
    char *ename;
    char *key;                    /* key for select_srv() */
    int packed;
    HDF *hdfsnd;
    unsigned char *payload;
    size_t psize;

    unsigned char *rcvbuf;
    HDF *hdfrcv;
} mevent_t;

typedef void (*MeventLog)(const char *func, const char *file, long line, int level, const char *format, ...);

/*
 * 内部使用
 */
unsigned int srv_get_msg_offset(struct mevent_srv *srv);
struct mevent_srv *select_srv(mevent_t *evt, const char *key, size_t ksize);

/*
 * init mevent's error
 */
MeventLog mevent_log;

NEOERR *merr_init(MeventLog logf);

/*
 * 初始化数据结构，用过后请使用mevent_free() 释放内存
 */
mevent_t *mevent_init(char *ename);
void mevent_free(void *evt);

/*
 * 添加后端通信服务器。对evt结构体添加数据，触发事件等操作之前必须调用此函数。
 */
int mevent_add_tipc_server(mevent_t *evt, int port);
int mevent_add_tcp_server(mevent_t *evt, const char *addr, int port,
                          const char *nblock, void *tv);
int mevent_add_udp_server(mevent_t *evt, const char *addr, int port,
                          const char *nblock, void *tv);
int mevent_add_sctp_server(mevent_t *evt, const char *addr, int port);


/*
 * 完成以上二组函数的功能: 初始化, 从配置文件中添加服务器.
 * 初始化: mevent_init()
 * 添加服务器: mevent_add_*_server()
 * 该函数会从配置文件中读取所有 ename 功能的服务器列表, 初始化,
 * 原来的 mevent_add_xxx_server() 可以继续在返回的 mevent_t 结构体下使用
 */
mevent_t *mevent_init_plugin(char *ename);


/*
 * 发包，触发事件中心处理。
 * 因为需要支持设置不同参数的循环使用，故每次trigger时会清空 hdfsnd 中的数据.
 * 不可以同一次参数设置循环调用
 * key: 用来选择处理后端的关键字（如UIN等），提供的话可以有效避免缓存冗余，可以为NULL
 * cmd: 命令号，不可重复使用，必填
 * flags: 请求标志，不可重复使用，必填
 * 返回值为该操作返回码, 分为三段区间, 取值范围参考 net-const.h 中 REP_xxx
 * 如果服务业务端有其他数据返回时, 返回数据存储在 evt->rcvdata 中
 */
int mevent_trigger(mevent_t *evt, char *key,
                   unsigned short cmd, unsigned short flags);


#define MEVENT_TRIGGER(evt, key, cmd, flags)                            \
    do {                                                                \
        if (PROCESS_NOK(mevent_trigger(evt, key, cmd, flags))) {        \
            char *msg = hdf_get_value(evt->hdfrcv, PRE_ERRMSG, NULL);   \
            char *trace = hdf_get_value(evt->hdfrcv, PRE_ERRTRACE, NULL); \
            NEOERR *e = nerr_raise(evt->errcode, msg);                  \
            return nerr_pass_ctx(e, trace);                             \
        }                                                               \
    } while(0)
#define MEVENT_TRIGGER_VOID(evt, key, cmd, flags)                       \
    do {                                                                \
        if (PROCESS_NOK(mevent_trigger(evt, key, cmd, flags))) {        \
            char *zpa = NULL;                                           \
            hdf_write_string(evt->hdfrcv, &zpa);                        \
            if (mevent_log) mevent_log(__PRETTY_FUNCTION__,__FILE__,__LINE__, 2, \
                                       "pro %s %d failure %d %s",       \
                                       evt->ename, cmd, evt->errcode, zpa); \
            if (zpa) free(zpa);                                         \
            return;                                                     \
        }                                                               \
    } while(0)
#define MEVENT_TRIGGER_RET(ret, evt, key, cmd, flags)                   \
    do {                                                                \
        if (PROCESS_NOK(mevent_trigger(evt, key, cmd, flags))) {        \
            char *zpa = NULL;                                           \
            hdf_write_string(evt->hdfrcv, &zpa);                        \
            if (mevent_log) mevent_log(__PRETTY_FUNCTION__,__FILE__,__LINE__, 2, \
                                       "pro %s %d failure %d %s",       \
                                       evt->ename, cmd, evt->errcode, zpa); \
            if (zpa) free(zpa);                                         \
            return ret;                                                 \
        }                                                               \
    } while(0)
#define MEVENT_TRIGGER_NRET(evt, key, cmd, flags)                       \
    do {                                                                \
        if (PROCESS_NOK(mevent_trigger(evt, key, cmd, flags))) {        \
            char *zpa = NULL;                                           \
            hdf_write_string(evt->hdfrcv, &zpa);                        \
            if (mevent_log) mevent_log(__PRETTY_FUNCTION__,__FILE__,__LINE__, 2, \
                                       "pro %s %d failure %d %s",       \
                                       evt->ename, cmd, evt->errcode, zpa); \
            if (zpa) free(zpa);                                         \
        }                                                               \
    } while(0)


#endif    /* __MEVENT_H__ */
