#include "udp.h"

int udp_init()
{
	int fd, rv;

	struct sockaddr_in srvsa;
	struct in_addr ia;
	if (inet_pton(AF_INET, "172.10.7.204", &ia) <= 0) return -1;
	srvsa.sin_family = AF_INET;
	srvsa.sin_addr.s_addr = ia.s_addr;
	srvsa.sin_port = htons(54110);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) return -1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &rv, sizeof(rv)) < 0) {
		close(fd);
		return -1;
	}

	if (bind(fd, (struct sockaddr*)&srvsa, sizeof(srvsa)) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

#define SBSIZE (68 * 1024)
static unsigned char static_buf[SBSIZE];

void udp_recv(int fd, short event, void *arg)
{
	struct sockaddr_in clisa;
	socklen_t clilen;
	char buf[1024] = "reply";
	int blen = 200;

	clilen = sizeof(clisa);

	if (recvfrom(fd, static_buf, SBSIZE, 0, (struct sockaddr*)&clisa, &clilen) < 8)
		return;

	//fprintf(stderr, "recved %s\n", static_buf);

	sendto(fd, buf, blen, 0, (struct sockaddr*)&clisa, clilen);
	static_buf[0] = '\0';
}

void udp_close(int fd)
{
	close(fd);
}

