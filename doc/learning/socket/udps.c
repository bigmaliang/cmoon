#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>        /* htonls() and friends */
#include <netinet/in.h>        /* INET stuff */
#include <netinet/udp.h>    /* UDP stuff */

#define SBSIZE (68 * 1024)
static unsigned char static_buf[SBSIZE];


void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
    int fd, rv;
    struct sockaddr_in srvsa;
    struct in_addr ia;

    //rv = inet_pton(AF_INET, "127.0.0.1", &ia);
    rv = inet_pton(AF_INET, "0.0.0.0", &ia);
    if (rv <= 0)
        return -1;

    srvsa.sin_family = AF_INET;
    srvsa.sin_addr.s_addr = ia.s_addr;
    srvsa.sin_port = htons(54110);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return -1;

    rv = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &rv, sizeof(rv)) < 0 ) {
        close(fd);
        return -1;
    }

    rv = bind(fd, (struct sockaddr *) &srvsa, sizeof(srvsa));
    if (rv < 0) {
        close(fd);
        return -1;
    }

    struct sockaddr_in clisa;
    socklen_t clilen;
    clilen = sizeof(clisa);
    char s[INET6_ADDRSTRLEN];

    char buf[1024] = "reply";
    int blen = 200;
    //rv = recvfrom(fd, static_buf, SBSIZE, 0, (struct sockaddr *) &clisa, &clilen);
    while ( (rv = recvfrom(fd, static_buf, SBSIZE, 0, (struct sockaddr *) &clisa, &clilen)) > 0) {
#if 0
        int count = 1;
        printf("listener: got packet from %s\n",
               inet_ntop(clisa.sin_family, get_in_addr((struct sockaddr *)&clisa), s, sizeof s));
        printf("listener: packet is %d bytes long\n", rv);
        static_buf[rv] = '\0';
        printf("listener: packet contains \"%s\"\n", static_buf);

        while (count > 0) {
#endif
            sendto(fd, buf, blen, 0, (struct sockaddr *) &clisa, clilen);
#if 0
            count--;
        }
#endif
        //sleep(3);
    }
    perror("recvfrom");
    
    close(fd);
    return 0;
}
