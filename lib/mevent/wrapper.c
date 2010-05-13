
#include <sys/types.h>	/* socket defines */
#include <sys/socket.h>	/* socket functions */
#include <stdlib.h>		/* malloc() */
#include <stdint.h>		/* uint32_t and friends */
#include <stdbool.h>	/* bool, true, false */
#include <arpa/inet.h>	/* htonls() and friends */
#include <string.h>		/* memcpy() */
#include <unistd.h>		/* close() */

#include "mevent.h"
#include "net-const.h"
#include "tipc.h"
#include "tcp.h"
#include "udp.h"
#include "sctp.h"
#include "internal.h"
#include "netutils.h"
#include "packet.h"

#define CONFIG_FILE		"/etc/mevent/client.hdf"

static bool loaded = false;
static HDF *g_cfg;

static int load_config()
{
	NEOERR *err;
	
	if (g_cfg != NULL) {
		hdf_destroy(&g_cfg);
	}
	err = hdf_init(&g_cfg);
	if (err != STATUS_OK) {
		nerr_ignore(&err);
		return 0;
	}

	err = hdf_read_file(g_cfg, CONFIG_FILE);
	if (err != STATUS_OK) {
		nerr_ignore(&err);
		return 0;
	}

	loaded = true;
	return 1;
}

mevent_t* mevent_init_plugin(char *ename, unsigned short cmd,
							 unsigned short flags)
{
	if (ename == NULL)
		return NULL;
	
	if (!loaded || g_cfg == NULL) {
		if (load_config() != 1) {
			return NULL;
		}
	}

	HDF *node = hdf_get_obj(g_cfg, ename);
	if (node == NULL)
		return NULL;
	
	mevent_t *evt = mevent_init();
	if (evt == NULL) {
		return NULL;
	}

	node = hdf_obj_child(node);

	char *type, *ip, *nblock;
	int port;
	struct timeval tv;
	while (node != NULL) {
		type = hdf_get_value(node, "type", "unknown");
		ip = hdf_get_value(node, "ip", "127.0.0.1") ;
		port = hdf_get_int_value(node, "port", 26010);
		nblock = hdf_get_value(node, "non_block", NULL);
		tv.tv_sec = hdf_get_int_value(node, "net_timeout_s", 0);
		tv.tv_usec = hdf_get_int_value(node, "net_timeout_u", 0);
			
		if (!strcmp(type, "tcp")) {
			mevent_add_tcp_server(evt, ip, port, nblock, &tv);
		} else if (!strcmp(type, "udp")) {
			mevent_add_udp_server(evt, ip, port, nblock, &tv);
		} else if (!strcmp(type, "tipc")) {
			mevent_add_tipc_server(evt, port);
		} else if (!strcmp(type, "sctp")) {
			mevent_add_sctp_server(evt, ip, port);
		} else {
			continue;
		}

		node = hdf_obj_next(node);
	}

	if (evt->nservers <= 0) {
		mevent_free(evt);
		return NULL;
	}

	char s[64]; size_t ksize;
	ksize = strlen(ename);
	neo_rand_string(s, 60);
	evt->ename = strdup(s);

	struct mevent_srv *srv = select_srv(evt, evt->ename, strlen(evt->ename));
	if (srv == NULL) {
		mevent_free(evt);
		return NULL;
	}

	unsigned int moff = srv_get_msg_offset(srv);

	unsigned char *p = evt->payload + moff;
	* (uint32_t *) p = htonl( (PROTO_VER << 28) | ID_CODE );
	* ((uint16_t *) p + 2) = htons(cmd);
	* ((uint16_t *) p + 3) = htons(flags);
	* ((uint32_t *) p + 2) = htonl(ksize);
	memcpy(p+12, ename, ksize);

	evt->psize = moff + 12 +ksize;
	evt->packed = 0;
	hdf_destroy(&evt->hdfsnd);
	hdf_init(&evt->hdfsnd);

	return evt;
}

