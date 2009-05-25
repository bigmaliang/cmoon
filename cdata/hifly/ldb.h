#ifndef __LDB_H__
#define __LDB_H__
#include "mheads.h"

__BEGIN_DECLS

#ifdef RELEASE
#define NMDB_SERVER_USER	"192.168.1.9"
#define NMDB_SERVER_PHOTO	"192.168.1.9"
#define NMDB_SERVER_VIDEO	"192.168.1.9"
#define NMDB_SERVER_BLOG	"192.168.1.9"
#define DB_IP			"192.168.1.24"
#else
#define NMDB_SERVER_USER	"192.168.8.54"
#define NMDB_SERVER_PHOTO	"192.168.8.54"
#define NMDB_SERVER_VIDEO	"192.168.8.54"
#define NMDB_SERVER_BLOG	"192.168.8.54"
#define DB_IP			"192.168.8.84"
#endif

#define NMDB_PORT_USER		26010
#define NMDB_PORT_PHOTO		26020
#define NMDB_PORT_VIDEO		26030
#define NMDB_PORT_BLOG		26040

#define DB_USER		"root"
#define DB_PASS		"hifly1234"

int ldb_init(fdb_t **fdb, char *ip, char *name);

void ldb_opfinish(int ret, HDF *hdf, mdb_conn *conn,
				  char *target, char *url, bool header);
void ldb_opfinish_json(int ret, HDF *hdf, mdb_conn *conn);

__END_DECLS
#endif	/* __LDB_H__ */
