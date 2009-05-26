#ifndef __LDB_H__
#define __LDB_H__
#include "mheads.h"

__BEGIN_DECLS

#ifdef RELEASE
#define NMDB_SERVER_USER	"127.0.0.1"
#define NMDB_SERVER_PHOTO	"127.0.0.1"
#define NMDB_SERVER_VIDEO	"127.0.0.1"
#define NMDB_SERVER_BLOG	"127.0.0.1"
#define DB_IP				"127.0.0.1"
#else
#define NMDB_SERVER_USER	"127.0.0.1"
#define NMDB_SERVER_PHOTO	"127.0.0.1"
#define NMDB_SERVER_VIDEO	"127.0.0.1"
#define NMDB_SERVER_BLOG	"127.0.0.1"
#define DB_IP				"127.0.0.1"
#endif

#define NMDB_PORT_USER		26010
#define NMDB_PORT_PHOTO		26010
#define NMDB_PORT_VIDEO		26010
#define NMDB_PORT_BLOG		26010

#define DB_USER		"test"
#define DB_PASS		"test"

int ldb_init(fdb_t **fdb, char *ip, char *name);

void ldb_opfinish(int ret, HDF *hdf, mdb_conn *conn,
				  char *target, char *url, bool header);
void ldb_opfinish_json(int ret, HDF *hdf, mdb_conn *conn);

__END_DECLS
#endif	/* __LDB_H__ */
