#ifndef __OGROUP_H__
#define __OGROUP_H__
#include "mheads.h"

__BEGIN_DECLS

int group_get_info(mdb_conn *conn, int gid, group_t **group);
int group_get_groups(HDF *hdf, mdb_conn *conn, session_t *ses);

__END_DECLS
#endif /* __OGROUP_H__ */
