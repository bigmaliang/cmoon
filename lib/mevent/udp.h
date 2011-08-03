
#ifndef _UDP_H
#define _UDP_H

int udp_srv_send(struct mevent_srv *srv, unsigned char *buf, size_t bsize);
uint32_t udp_get_rep(struct mevent_srv *srv,
                     unsigned char *buf, size_t bsize,
                     unsigned char **payload, size_t *psize);

#endif

