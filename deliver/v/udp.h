#ifndef __UDP_H__
#define __UDP_H__

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <event.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>		/* htonls() and friends */
#include <netinet/in.h>		/* INET stuff */

int udp_init();
void udp_recv(int fd, short event, void *arg);
void udp_close(int fd);

#endif
