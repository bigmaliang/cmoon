#ifndef __HONGHONG_H__
#define __HONGHONG_H__
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

int admin_group_data_get(CGI *cgi, HASH *dbh, session_t *ses);
int admin_group_data_mod(CGI *cgi, HASH *dbh, session_t *ses);
int admin_group_data_add(CGI *cgi, HASH *dbh, session_t *ses);
int admin_group_data_del(CGI *cgi, HASH *dbh, session_t *ses);


int member_login_data_get(CGI *cgi, HASH *dbh, session_t *ses);
int member_logout_data_get(CGI *cgi, HASH *dbh, session_t *ses);
int member_regist_data_get(CGI *cgi, HASH *dbh, session_t *ses);


int service_action_data_get(CGI *cgi, HASH *dbh, session_t *ses);


int static_csc_data_get(CGI *cgi, HASH *dbh);


int csc_data_get(CGI *cgi, HASH *dbh, session_t *ses);

__END_DECLS
#endif	/* __HONGHONG_H__ */
