
#ifndef _TIPC_H
#define _TIPC_H

int tipc_srv_send(struct mevent_srv *srv, const unsigned char *buf, size_t bsize);
uint32_t tipc_get_rep(struct mevent_srv *srv,
                      unsigned char *buf, size_t bsize,
                      unsigned char **payload, size_t *psize);

#endif

