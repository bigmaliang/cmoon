#ifndef __COMMENT_H__
#define __COMMENT_H__

#include "mheads.h"

__BEGIN_DECLS

int comment_get(HDF *hdf, mdb_conn *conn);
int comment_add(HDF *hdf, mdb_conn *conn);

__END_DECLS
#endif	/* __COMMENT_H__ */
