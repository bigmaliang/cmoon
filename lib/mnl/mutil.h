#ifndef __MUTIL_H__
#define __MUTIL_H__

#include "mheads.h"

__BEGIN_DECLS

#define REQ_GET		"GET"
#define REQ_PUT		"PUT"
#define REQ_POST	"POST"
#define REQ_DEL		"DELETE"

/*
 * With PUT and DELETE, there is a big difference between zero request and one,
 * but there is no difference between one request and ten.
 */
#define REQ_IS_GET(op)	(!strcmp(op, REQ_GET))
#define REQ_IS_PUT(op)	(!strcmp(op, REQ_PUT))
#define REQ_IS_POST(op)	(!strcmp(op, REQ_POST))
#define REQ_IS_DEL(op)	(!strcmp(op, REQ_DEL))

enum cgi_req_method {
	CGI_REQ_GET = 0,
	CGI_REQ_PUT,
	CGI_REQ_POST,
	CGI_REQ_DEL,
	CGI_REQ_UNKNOWN
};

int CGI_REQ_METHOD(CGI *cgi);

#define TGT_PARENT	"parent"
#define TGT_OPENNER	"opener"
#define TGT_TOP		"top"
#define TGT_SELF	"self"

#define URL_BLANK	"location.href='/blank.html'"
#define URL_RELOAD	"location.reload()"
#define URL_CLOSE	"close()"
#define URL_BACK	"history.back(-1)"
#define URL_HREF	""


/* res must have enough capacity */
bool mutil_client_attack(HDF *hdf, char *action, int limit, time_t exp);

void mutil_redirect(const char *msg, const char *target, const char *url, bool header);

char* mutil_hdf_attr(HDF *hdf, char *name, char*key);
char* mutil_obj_attr(HDF *hdf, char*key);
bool mutil_isdigit(char *s);
void mutil_wrap_fcgi(int argc, char **argv, char **envp);

__END_DECLS
#endif	/* __MUTIL_H__ */
