#ifndef __LUTIL_H__
#define __LUTIL_H__
#include "mheads.h"

__BEGIN_DECLS

#define UIN_ILLEGAL(u)	(u < MIN_USER_NUM)
#define GID_ILLEGAL(g)	(g < 0)

enum cgi_req_type {
	CGI_REQ_HTML = 0,
	CGI_REQ_AJAX,
	CGI_REQ_UNSUPPORT
};
int CGI_REQ_TYPE(CGI *cgi, HDF *rcfg);

void* lutil_get_data_handler(void *lib, CGI *cgi, HDF *rcfg);
int lutil_fetch_count(HDF *hdf, mdb_conn *conn, char *table, char *cond);
int lutil_fetch_countf(HDF *hdf, mdb_conn *conn, char *table, char *cfmt, ...);

__END_DECLS
#endif	/* __LUTIL_H__ */
