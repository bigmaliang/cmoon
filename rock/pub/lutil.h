#ifndef __LUTIL_H__
#define __LUTIL_H__
#include "mheads.h"

__BEGIN_DECLS

#define UIN_ILLEGAL(u)	(u < MIN_USER_NUM)

enum cgi_req_type {
	CGI_REQ_HTML = 0,
	CGI_REQ_AJAX,
	CGI_REQ_UNSUPPORT
};
int CGI_REQ_TYPE(CGI *cgi);

int lutil_file_check_power(CGI *cgi, mdb_conn *conn, char *uri, bool split);
int lutil_file_access(CGI *cgi, mdb_conn *conn);
int lutil_file_access_rewrited(CGI *cgi, HASH *dbh);
void* lutil_get_data_handler(void *lib, CGI *cgi);
int lutil_render(CGI *cgi, HASH *tplh);

int  lutil_init_db(HASH **dbh);
void lutil_cleanup_db(HASH *dbh);
int  tpl_config(const struct dirent *ent);
int  lutil_init_tpl(HASH **tplh);
void lutil_cleanup_tpl(HASH *tplh);
bool lutil_makesure_dir(char *file);

__END_DECLS
#endif	/* __LUTIL_H__ */
