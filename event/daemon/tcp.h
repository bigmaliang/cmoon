
#ifndef _TCP_H
#define _TCP_H

int tcp_init(void);
void tcp_close(int fd);
void tcp_newconnection(int fd, short event, void *arg);

#endif

