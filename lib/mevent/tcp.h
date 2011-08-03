
#ifndef _TCP_H
#define _TCP_H

int tcp_srv_send(struct mevent_srv *srv, unsigned char *buf, size_t bsize);
uint32_t tcp_get_rep(struct mevent_srv *srv,
                     unsigned char *buf, size_t bsize,
                     unsigned char **payload, size_t *psize);

#endif
