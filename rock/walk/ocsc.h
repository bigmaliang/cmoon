#ifndef __OCSC_H__
#define __OCSC_H__
#include "mheads.h"

__BEGIN_DECLS

#define IMG_PATH	"csc"

int csc_get_data(HDF *hdf, HASH *dbh, session_t *ses);
void csc_refresh_info(int fid);
int csc_add_image(CGI *cgi, mdb_conn *conn, session_t *ses);
int csc_add_item(HDF *hdf, mdb_conn *conn, session_t *ses);

__END_DECLS
#endif /* __OCSC_H__ */
