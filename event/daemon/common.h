
/* Global data used throughout the whole application. */

#ifndef _COMMON_H
#define _COMMON_H

/* The queue for database operations */
#include "queue.h"
#include "mevent.h"

extern struct mevent *mevent;

/* Settings */
struct settings {
	int tipc_lower;
	int tipc_upper;
	char *tcp_addr;
	int tcp_port;
	char *udp_addr;
	int udp_port;
	char *sctp_addr;
	int sctp_port;
	int foreground;
	int passive;
	char *logfname;
	int trace_level;
	char *conffname;
};
extern struct settings settings;

/* Statistics */
#include "stats.h"
extern struct stats stats;

#include "ClearSilver.h"
extern HDF *g_cfg;

#endif

