#ifndef __OAPP_H__
#define __OAPP_H__
#include "mheads.h"

__BEGIN_DECLS

int app_get_info_by_id(mdb_conn *conn, int id, char *url, int pid);
int app_new_data_get(CGI *cgi, HASH *dbh, HASH *evth, session_t *ses);


__END_DECLS
#endif /* __OAPP_H__ */
