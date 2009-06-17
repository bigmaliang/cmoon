#ifndef __ADMIN_H__
#define __ADMIN_H__
#include "mheads.h"

__BEGIN_DECLS

int admin_account_data_get(CGI *cgi, HASH *dbh, session_t *ses);
int admin_account_data_mod(CGI *cgi, HASH *dbh, session_t *ses);
int admin_account_data_add(CGI *cgi, HASH *dbh, session_t *ses);
int admin_account_data_del(CGI *cgi, HASH *dbh, session_t *ses);

int admin_file_data_get(CGI *cgi, HASH *dbh, session_t *ses);
int admin_file_data_mod(CGI *cgi, HASH *dbh, session_t *ses);
int admin_file_data_add(CGI *cgi, HASH *dbh, session_t *ses);
int admin_file_data_del(CGI *cgi, HASH *dbh, session_t *ses);

__END_DECLS
#endif	/* __ADMIN_H__ */
