#ifndef __FUTIL_H__
#define __FUTIL_H__

#define TGT_PARENT	"parent"
#define TGT_OPENNER	"opener"
#define TGT_TOP		"top"
#define TGT_SELF	"self"

#define URL_BLANK	"location.href='/blank.html'"
#define URL_RELOAD	"location.reload()"
#define URL_CLOSE	"close()"
#define URL_BACK	"history.back(-1)"
#define URL_HREF	""

int cgi_query_method(CGI *cgi);
char* futil_hdf_attr(HDF *hdf, char *name, char*key);
char* futil_obj_attr(HDF *hdf, char*key);
bool futil_getdatetime(char *res, int len, const char *fmt, time_t second);
bool futil_isdigit(char *s);
bool futil_is_userdata_key(char *key);
void futil_redirect(const char *msg, const char *target, const char *url, bool header);
void futil_wrap_fcgi(int argc, char **argv, char **envp);

#endif	/* __FUTIL_H__ */
