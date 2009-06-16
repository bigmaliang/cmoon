#ifndef __MEMBER_H__
#define __MEMBER_H__
#include "mheads.h"

__BEGIN_DECLS

int member_login_data_get(CGI *cgi, HASH *dbh);
int member_logout_data_get(CGI *cgi, HASH *dbh);
int member_regist_data_get(CGI *cgi, HASH *dbh);

__END_DECLS
#endif	/* __MEMBER_H__ */
