
#include <sys/types.h>	/* socket defines */
#include <sys/socket.h>	/* socket functions */
#include <stdlib.h>		/* malloc() */
#include <stdint.h>		/* uint32_t and friends */
#include <stdbool.h>	/* bool, true, false */
#include <arpa/inet.h>	/* htonls() and friends */
#include <string.h>		/* memcpy() */
#include <unistd.h>		/* close() */

#include "mevent.h"
#include "net-const.h"
#include "tipc.h"
#include "tcp.h"
#include "udp.h"
#include "sctp.h"
#include "internal.h"
#include "netutils.h"
#include "packet.h"


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

/* Creates a mevent_t. */
mevent_t *mevent_init(void)
{
	mevent_t *evt;
	NEOERR *err;

	evt = malloc(sizeof(mevent_t));
	if (evt == NULL) {
		return NULL;
	}

	evt->servers = NULL;
	evt->nservers = 0;
	evt->ename = NULL;
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

/* Frees a mevent_t structure created with mevent_init(). */
int mevent_free(mevent_t *evt)
{
    if (evt == NULL) return 0;
    
	if (evt->servers != NULL) {
		int i;
		for (i = 0; i < evt->nservers; i++)
			close(evt->servers[i].fd);
		free(evt->servers);
	}
	if (evt->ename != NULL)
		free(evt->ename);
	hdf_destroy(&evt->hdfsnd);
	if (evt->payload != NULL)
		free(evt->payload);
	if (evt->rcvbuf != NULL)
		free(evt->rcvbuf);
	hdf_destroy(&evt->hdfrcv);
	free(evt);
	return 1;
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

/*
 * chose_plugin will reset the evt->psize, equal clear the varaialbel
 * seted formerly. so, call it first
 */
int mevent_chose_plugin(mevent_t *evt, const char *key,
						unsigned short cmd, unsigned short flags)
{
	struct mevent_srv *srv;
	unsigned char *p;
	size_t ksize;

	if (key == NULL || evt == NULL ||
	    strlen(key) == 0 || strlen(key) > MAX_PACKET_LEN-20) return 0;

	if (evt->ename == NULL) {
		evt->ename = strdup(key);
	}

	ksize = strlen(evt->ename);
	srv = select_srv(evt, evt->ename, ksize);
    
	if (srv == NULL) return 0;
	unsigned int moff = srv_get_msg_offset(srv);

    ksize = strlen(key);
	p = evt->payload + moff;
	* (uint32_t *) p = htonl( (PROTO_VER << 28) | ID_CODE );
	* ((uint16_t *) p + 2) = htons(cmd);
	* ((uint16_t *) p + 3) = htons(flags);
	* ((uint32_t *) p + 2) = htonl(ksize);
	memcpy(p+12, key, ksize);

	evt->psize = moff + 12 +ksize;
	evt->packed = 0;
	hdf_destroy(&evt->hdfsnd);
	hdf_init(&evt->hdfsnd);

	return 1;
}

int mevent_trigger(mevent_t *evt)
{
	size_t t, ksize, vsize;
	struct mevent_srv *srv;
	unsigned char *p;
	uint32_t rv;
	
	if (evt == NULL || evt->psize < 13 ||
	    (evt->psize+4) > MAX_PACKET_LEN) return 0;
	
	ksize = strlen(evt->ename);
	//if (ksize == 0) return 0;

	if (!evt->packed) {
		vsize = pack_hdf(evt->hdfsnd, evt->payload + evt->psize);
		evt->psize += vsize;
		evt->packed = 1;
	}

	if (evt->psize < 17) {
		* (uint32_t *) (evt->payload+evt->psize) = htonl(DATA_TYPE_EOF);
		evt->psize += sizeof(uint32_t);
	}
	
	srv = select_srv(evt, evt->ename, ksize);
	if (srv == NULL) return 0;
	
	t = srv_send(srv, evt->payload, evt->psize);
	if (t <= 0) {
		return -1;
	}

	vsize = 0;
	rv = get_rep(srv, evt->rcvbuf, MAX_PACKET_LEN, &p, &vsize);

	if (vsize > 8) {
		/* reply_long add a vsize parameter */
		unpack_hdf(p+4, vsize-4, &evt->hdfrcv);
	}

	hdf_destroy(&evt->hdfsnd);
	hdf_init(&evt->hdfsnd);
	
	return rv;
}
