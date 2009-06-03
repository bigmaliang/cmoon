#ifndef __LDB_H__
#define __LDB_H__
#include "mheads.h"

__BEGIN_DECLS

int ldb_init(fdb_t **fdb, char *ip, char *name, unsigned int port);

void ldb_opfinish(int ret, HDF *hdf, mdb_conn *conn,
				  char *target, char *url, bool header);
void ldb_opfinish_json(int ret, HDF *hdf, mdb_conn *conn);

__END_DECLS
#endif	/* __LDB_H__ */
