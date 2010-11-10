#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>		/* htonls() and friends */
#include <netinet/in.h>		/* INET stuff */
#include <netinet/udp.h>	/* UDP stuff */

int main()
{
	int fd;
	struct timeval tv;
	tv.tv_sec = 0; tv.tv_usec = 200;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return 0;

	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));

	struct in_addr ia;
	inet_pton(AF_INET, "127.0.0.1", &ia);

	struct sockaddr_in srvsa;
	srvsa.sin_family = AF_INET;
	srvsa.sin_port = htons(54110);
	srvsa.sin_addr.s_addr = ia.s_addr;
	socklen_t srvlen = sizeof(struct sockaddr_in);

	char buf[1024] = "hello man", rcv[1024] = {};
	int bsize = 200;
	ssize_t rv;
	rv = sendto(fd, buf, bsize, 0,
				(struct sockaddr *) &srvsa,	srvlen);

	//printf("%d", rv);

	int count = 0;
	while (count < 2 && (rv = recv(fd, rcv, bsize, 0)) > 1) {
		//rv = recv(fd, rcv, bsize, 0);
		//if (rv > 0)
		printf("%d %d, %s\n", count, rv, rcv);
			count++;
	}

	sleep(3);

	rv = sendto(fd, buf, bsize, 0,
				(struct sockaddr *) &srvsa,	srvlen);
	while ((rv = recv(fd, rcv, bsize, 0)) > 1) {
		//rv = recv(fd, rcv, bsize, 0);
		//if (rv > 0)
		printf("%d %d, %s\n", count, rv, rcv);
			count++;
	}
	
	close(fd);
	
	return 0;
}
