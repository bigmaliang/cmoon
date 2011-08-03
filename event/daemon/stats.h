
#ifndef _STATS_H
#define _STATS_H

/* Statistics structure */
struct stats {
    unsigned long msg_tipc;
    unsigned long msg_tcp;
    unsigned long msg_udp;
    unsigned long msg_sctp;

    unsigned long net_version_mismatch; /* 5 */
    unsigned long net_broken_req;
    unsigned long net_unk_req;

    unsigned long pro_busy;
};

#define STATS_REPLY_SIZE 8

void stats_init(struct stats *s);

#endif

