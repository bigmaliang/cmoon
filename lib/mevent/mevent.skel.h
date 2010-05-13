
/* Header for the libmevent library. */

#ifndef _MEVENT_H
#define _MEVENT_H

#include <time.h>
#include "net-const.h"

#include "ClearSilver.h"

/* Defined to 0 or 1 at libmevent build time according the build configuration,
 * not to be used externally. */
#define _ENABLE_TIPC ++CONFIG_ENABLE_TIPC++
#define _ENABLE_TCP ++CONFIG_ENABLE_TCP++
#define _ENABLE_UDP ++CONFIG_ENABLE_UDP++
#define _ENABLE_SCTP ++CONFIG_ENABLE_SCTP++


#include <sys/types.h>		/* socket defines */
#include <sys/socket.h>		/* socklen_t */

#if _ENABLE_TIPC
#include <linux/tipc.h>		/* struct sockaddr_tipc */
#endif

#if (_ENABLE_TCP || _ENABLE_UDP || _ENABLE_SCTP)
#include <netinet/in.h>		/* struct sockaddr_in */
#endif

struct mevent_srv {
	int fd;
	int type;
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
	char *ename;
	int packed;
	HDF *hdfsnd;
	unsigned char *payload;
	size_t psize;
	
	unsigned char *rcvbuf;
	HDF *hdfrcv;
} mevent_t;

/*
 * 内部使用
 */
unsigned int srv_get_msg_offset(struct mevent_srv *srv);
struct mevent_srv *select_srv(mevent_t *evt, const char *key, size_t ksize);

/*
 * 初始化数据结构，用过后请使用mevent_free() 释放内存
 */
mevent_t *mevent_init();
int mevent_free(mevent_t *evt);

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
 * 选择后端处理插件，该函数会清空evt中已经添加的数据。
 * cmd, flags 用来指定插件处理的模式。详细选择请参考event/base/net-const.h 中的宏定义
 */
int mevent_chose_plugin(mevent_t *evt, const char *key,
						unsigned short cmd, unsigned short flags);


/*
 * 完成以上三组函数的功能: 初始化, 添加服务器, 选择插件.
 * 初始化: mevent_init()
 * 添加服务器: mevent_add_*_server()
 * 选择插件: mevent_chose_plugin()
 * 该函数会从配置文件中读取所有 ename 功能的服务器列表, 初始化,
 * 并随机选择一台作为通信后台
 * 原来的 mevent_add_xxx_server(), mevent_chose_plugin() 可以
 * 继续在返回的 mevent_t 结构体下使用
 */
mevent_t* mevent_init_plugin(char *ename, unsigned short cmd,
							 unsigned short flags);


/*
 * 发包，触发事件中心处理。不会清空evt中的数据，可以循环调用
 * 返回值为该操作返回码, 分为三段区间, 取值范围参考 net-const.h 中 REP_xxx
 * 如果服务业务端有其他数据返回时, 返回数据存储在 evt->rcvdata 中
 */
int mevent_trigger(mevent_t *evt);

#endif	/* __MEVENT_H__ */

