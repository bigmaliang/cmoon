#ifndef __LUTIL_H__
#define __LUTIL_H__
#include "mheads.h"

__BEGIN_DECLS

#define UIN_ILLEGAL(u)	(u < MIN_USER_NUM)

int check_user_power(CGI *cgi, mdb_conn *conn, file_t *file, int access);
file_t* lutil_get_file(mdb_conn *conn, char *url, int pid);
int lutil_get_files(mdb_conn *conn, ULIST *urls, ULIST **files, int *noksn);

void lutil_file_access_json(CGI *cgi, mdb_conn *conn);

__END_DECLS
#endif	/* __LUTIL_H__ */
