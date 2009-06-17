#ifndef __MEMBER_H__
#define __MEMBER_H__
#include "mheads.h"

__BEGIN_DECLS

int member_login_data_get(CGI *cgi, HASH *dbh, session_t *ses);
int member_logout_data_get(CGI *cgi, HASH *dbh, session_t *ses);
int member_regist_data_get(CGI *cgi, HASH *dbh, session_t *ses);

__END_DECLS
#endif	/* __MEMBER_H__ */
