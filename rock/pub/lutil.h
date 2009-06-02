#ifndef __LUTIL_H__
#define __LUTIL_H__
#include "mheads.h"

__BEGIN_DECLS

#define UIN_ILLEGAL(u)	(u < MIN_USER_NUM)

int  lutil_file_check_power(CGI *cgi, mdb_conn *conn, char *uri);
void lutil_file_access_json(CGI *cgi, mdb_conn *conn);
int  lutil_file_access(CGI *cgi, HASH *dbh);


int  lutil_init_db(HASH **dbh);
void lutil_cleanup_db(HASH *dbh);

__END_DECLS
#endif	/* __LUTIL_H__ */
