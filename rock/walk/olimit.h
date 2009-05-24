#ifndef __OLIMIT_H__
#define __OLIMIT_H__
#include "mheads.h"

/*
 * essentinally, olimit should be in ofile,
 * but, we divide them for logically reason.
 */
__BEGIN_DECLS

int  limit_get_limits(HDF *hdf, mdb_conn *conn);
void limit_translate_mode(HDF *hdf);
int  limit_modify(HDF *hdf, mdb_conn *conn);
int  limit_add(HDF *hdf, mdb_conn *conn);
int  limit_delete(HDF *hdf, mdb_conn *conn);

__END_DECLS
#endif	/* __OLIMIT_H__ */
