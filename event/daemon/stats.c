
#include "stats.h"

void stats_init(struct stats *s)
{
    s->msg_tipc = 0;
    s->msg_tcp = 0;
    s->msg_udp = 0;
    s->msg_sctp = 0;

    s->net_version_mismatch = 0;
    s->net_broken_req = 0;
    s->net_unk_req = 0;

    s->pro_busy = 0;
}


