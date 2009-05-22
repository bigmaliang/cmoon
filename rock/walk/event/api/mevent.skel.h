
/* Header for the libmevent library. */

#ifndef _MEVENT_H
#define _MEVENT_H

#include "net-const.h"

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

#define MAX_PACKET_LEN	(68*1024)

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
	struct data_cell *dataset;
	unsigned char *payload;
	size_t psize;
	
	unsigned char *rcvbuf;
	struct data_cell *rcvdata;
} mevent_t;

/*
 * 初始化数据结构，用过后请使用mevent_free() 释放内存
 */
mevent_t *mevent_init();
int mevent_free(mevent_t *evt);

/*
 * 添加后端通信服务器。对evt结构体添加数据，触发事件等操作之前必须调用此函数。
 */
int mevent_add_tipc_server(mevent_t *evt, int port);
int mevent_add_tcp_server(mevent_t *evt, const char *addr, int port);
int mevent_add_udp_server(mevent_t *evt, const char *addr, int port);
int mevent_add_sctp_server(mevent_t *evt, const char *addr, int port);

/*
 * 选择后端处理插件，该函数会清空evt中已经添加的数据。
 * cmd, flags 用来指定插件处理的模式。详细选择请参考event/base/net-const.h 中的宏定义
 */
int mevent_chose_plugin(mevent_t *evt, const char *key,
			unsigned short cmd, unsigned short flags);

/*
 * 为相应插件添加数据。返回 1 表示成功， 0 表示失败
 * parent 表示数据添加到节点的名字，该名字必须为已经存在于evt结构体中的一个数组节点。
 * 否则添加失败。
 * parent 为 NULL 时，数据会添加到evt的根节点。
 */
int mevent_add_u32(mevent_t *evt, const char *parent, const char *key, uint32_t val);
int mevent_add_ulong(mevent_t *evt, const char *parent, const char *key, unsigned long val);
int mevent_add_str(mevent_t *evt, const char *parent, const char *key, const char *val);
int mevent_add_array(mevent_t *evt, const char *parent, const char *key);

/*
 * 发包，触发事件中心处理。不会清空evt中的数据，可以循环调用
 * 成功返回 REP_OK，其他非错误的特殊情况返回插件约定的相应标志，如好友已存在。
 * 处理错误返回 REP_ERR，errcode 存储错误码
 */
int mevent_trigger(mevent_t *evt, uint32_t *errcode);

#endif	/* __MEVENT_H__ */

