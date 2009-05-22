
#ifndef _MYTIPC_H
#define _MYTIPC_H

int tipc_init(void);
void tipc_close(int fd);
void tipc_recv(int fd, short event, void *arg);

#endif

