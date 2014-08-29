#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>        /* htonls() and friends */
#include <netinet/in.h>        /* INET stuff */
#include <netinet/udp.h>    /* UDP stuff */
#include "timer.h"

int main(int argc, char *argv[])
{
    int fd;
    
    if (argc < 2) {
        printf("%s ip\n", argv[0]);
        return -1;
    }
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return 0;

    struct in_addr ia;
    inet_pton(AF_INET, argv[1], &ia);

    struct sockaddr_in srvsa;
    srvsa.sin_family = AF_INET;
    srvsa.sin_port = htons(54110);
    srvsa.sin_addr.s_addr = ia.s_addr;
    socklen_t srvlen = sizeof(struct sockaddr_in);

    char buf[1024] = "hello man", rcv[1024] = {};
    int bsize = 200;
    ssize_t rv;

    struct timeval tv;
    tv.tv_sec = 0; tv.tv_usec = 20000;
    //setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
    //setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));

    //printf("%d", rv);

    unsigned long s_elapsed;
    int count = 0;
    timer_start();
    while (count < 400000) {
        rv = sendto(fd, buf, bsize, 0, (struct sockaddr *) &srvsa, srvlen);
        if (rv < 1) {
            perror("send");
            return 1;
        }
        
        rv = recv(fd, rcv, bsize, 0);
        if (rv > 1) {
            //printf("%d %d, %s\n", count, rv, rcv);
        } else {
            perror("rcv");
            return 1;
        }
        count++;
    }
    s_elapsed = timer_stop();
    printf("%lu\n", s_elapsed);

    close(fd);
    
    return 0;
}
