
#ifndef _INTERNAL_H
#define _INTERNAL_H

/* Different connection types. Used internally to differentiate between the
 * different protocols in struct mevent_srv. */
#define TIPC_CONN 1
#define TCP_CONN 2
#define UDP_CONN 3
#define SCTP_CONN 4

/* The ID code for requests is hardcoded for now, until asynchronous requests
 * are implemented. */
#define ID_CODE 1
extern unsigned int g_reqid;

/* For a given buffer, how much into it should the generic library code write
 * the message contents. */
#define TIPC_MSG_OFFSET 0
#define TCP_MSG_OFFSET 4
#define UDP_MSG_OFFSET 0
#define SCTP_MSG_OFFSET 0

/* Functions used internally but shared among the different files. */
int compare_servers(const void *s1, const void *s2);
ssize_t srecv(int fd, unsigned char *buf, size_t count, int flags);
ssize_t ssend(int fd, const unsigned char *buf, size_t count, int flags);

#endif

