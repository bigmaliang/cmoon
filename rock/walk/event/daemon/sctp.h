
#ifndef _SCTP_H
#define _SCTP_H

int sctp_init(void);
void sctp_close(int fd);
void sctp_recv(int fd, short event, void *arg);

#endif

