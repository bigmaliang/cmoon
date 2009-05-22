
#ifndef _UDP_H
#define _UDP_H

int udp_init(void);
void udp_close(int fd);
void udp_recv(int fd, short event, void *arg);

#endif

