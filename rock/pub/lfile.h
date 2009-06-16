#ifndef __LFILE_H__
#define __LFILE_H__
#include "mheads.h"

__BEGIN_DECLS

int lfile_access(CGI *cgi, mdb_conn *conn);
int lfile_access_rewrited(CGI *cgi, HASH *dbh);

__END_DECLS
#endif	/* __LFILE_H__ */
