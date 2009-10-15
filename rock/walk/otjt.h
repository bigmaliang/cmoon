#ifndef __OTJT_H__
#define __OTJT_H__
#include "mheads.h"

__BEGIN_DECLS

#define IMG_PATH	"tjt"

int tjt_get_data(HDF *hdf, HASH *dbh, session_t *ses);
void tjt_refresh_info(int aid, int fid);
int tjt_add_image(CGI *cgi, mdb_conn *conn, session_t *ses);
int tjt_add_atom(HDF *hdf, mdb_conn *conn, session_t *ses);

__END_DECLS
#endif /* __OTJT_H__ */
