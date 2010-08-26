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

#define TGT_PARENT	"parent."
#define TGT_OPENNER	"opener."
#define TGT_TOP		"top."
#define TGT_SELF	"self."

#define URL_BLANK	"location.href='/blank.html'"
#define URL_RELOAD	"location.reload()"
#define URL_CLOSE	"close()"
#define URL_BACK	"history.back(-1)"
#define URL_HREF	""


/* res must have enough capacity */
bool mutil_client_attack(HDF *hdf, char *action, uint64_t limit, time_t exp);
bool mutil_client_attack_cookie(HDF *hdf, char *action, uint64_t limit, time_t exp);

void mutil_redirect(const char *msg, const char *target, const char *url, bool header);

char* mutil_hdf_attr(HDF *hdf, char *name, char*key);
char* mutil_obj_attr(HDF *hdf, char*key);
bool mutil_isdigit(char *s);
bool mutil_makesure_dir(char *file);

/*
 * make sure buf big enough please
 * both alloc() and free() MUST be done by caller
 */
void mutil_real_escape_string(char *buf, char *val, size_t len);
/*
 * caller just need free() the *to;
 */
char* mutil_real_escape_string_nalloc(char **to, char *from, size_t len);
int  mutil_replace_dbint(char **sql, int val);
int  mutil_replace_dbstr(char **sql, char *val);
int  mutil_expand_strvf_dbfmt(char **str, const char *fmt, va_list ap);
int  mutil_expand_strvf(char **outstr, const char *sql_fmt, const char *fmt, va_list ap);
/*
 * expand
 * "UPDATE %s SET musn=$1, uin=$2 WHERE uin=%d", "s", &p, table, uin, musn, uinnew
 * to
 * "UPDATE tablexxx SET musn='test\'s member', uin=3333 WHERE uin=2323"
 *
 * defect: $[:digit:] is reserved by this function,
 * so, your sql's $[:digit:] will make mistake
 */
int  mutil_expand_strf(char **outstr, const char *sql_fmt, const char *fmt, ...);

__END_DECLS
#endif	/* __MUTIL_H__ */
