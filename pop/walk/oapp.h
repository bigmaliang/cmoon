#ifndef __OAPP_H__
#define __OAPP_H__
#include "mheads.h"

__BEGIN_DECLS

int app_get_info_by_id(mdb_conn *conn, int id, char *url, int pid);


__END_DECLS
#endif /* __OAPP_H__ */
