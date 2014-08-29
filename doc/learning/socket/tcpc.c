#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>        /* htonls() and friends */
#include <netinet/in.h>        /* INET stuff */
#include <netinet/udp.h>    /* UDP stuff */
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "timer.h"

int main(int argc, char *argv[])
{
    int fd;

    if (argc < 2) {
        printf("%s ip\n", argv[0]);
        return -1;
    }
    
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0)
        return 0;

    struct in_addr ia;
    inet_pton(AF_INET, argv[1], &ia);

    struct sockaddr_in srvsa;
    srvsa.sin_family = AF_INET;
    srvsa.sin_port = htons(54110);
    srvsa.sin_addr.s_addr = ia.s_addr;
    socklen_t srvlen = sizeof(struct sockaddr_in);

    if (connect(fd, (struct sockaddr*)&srvsa, srvlen) == -1) {
        perror("connect");
        return 1;
    }

    struct timeval tv;
    tv.tv_sec = 0; tv.tv_usec = 20000;
    //setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
    //setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));

    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &one, sizeof(one));
    int old_flags;
    
    old_flags = fcntl(fd, F_GETFL, 0);
    
    if (!(old_flags & O_NONBLOCK)) {
        old_flags |= O_NONBLOCK;
    }
    fcntl(fd, F_SETFL, old_flags);

    char buf[1024] = "hello man", rcv[1024] = {};
    int bsize = 200;
    ssize_t rv;

    //sleep(5);
    printf("begin\n");

    unsigned long s_elapsed;
    int count = 0;
    timer_start();
    while (count < 2000 ) {
        rv = send(fd, buf, bsize, 0);
        //printf("send RET %d %d\n", rv, count);
        if (rv < 1) {
            perror("send");
            return 1;
        }

        sleep(1);
        rv = recv(fd, rcv, bsize, 0);
        //printf("recv RET %d\n", rv);
        if (rv > 1) {
            //rv = recv(fd, rcv, bsize, 0);
            //if (rv > 0)
            //    printf("%d %d, %s\n", count, rv, rcv);
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
