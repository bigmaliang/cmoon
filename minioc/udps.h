#ifndef __UDPS_H__
#define __UDPS_H__

#include "apev.h"

int udps_init(char *ip, int port);
NEOERR* udps_recv(int fd, short event, void *arg);
void udps_close(int fd);

#endif
