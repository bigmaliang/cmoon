#ifndef __MUTIL_H__
#define __MUTIL_H__

#include "mheads.h"

__BEGIN_DECLS

/*
 * With PUT and DELETE, there is a big difference between zero request and one,
 * but there is no difference between one request and ten.
 */
enum cgi_req_method {
    CGI_REQ_GET = 0,
    CGI_REQ_PUT,
    CGI_REQ_POST,
    CGI_REQ_DEL,
    CGI_REQ_UNKNOWN
};

int CGI_REQ_METHOD(CGI *cgi);

#define TGT_PARENT    "parent."
#define TGT_OPENNER    "opener."
#define TGT_TOP        "top."
#define TGT_SELF    "self."

#define URL_BLANK    "location.href='/blank.html'"
#define URL_RELOAD    "location.reload()"
#define URL_CLOSE    "close()"
#define URL_BACK    "history.back(-1)"
#define URL_HREF    ""


/* res must have enough capacity */
bool mutil_client_attack(HDF *hdf, char *action, char *cname, uint64_t limit, time_t exp);
bool mutil_client_attack_cookie(HDF *hdf, char *action, uint64_t limit, time_t exp);

void mutil_redirect(const char *msg, const char *target, const char *url, bool header);

/* out must be 33 length */
void mutil_md5_str(char *in, char out[LEN_MD5]);
char* mutil_hdf_attr(HDF *hdf, char *name, char*key);
char* mutil_obj_attr(HDF *hdf, char*key);
bool mutil_isdigit(char *s);
NEOERR* mutil_makesure_dir(char *file);

/*
 * make sure buf big enough please
 * both alloc() and free() MUST be done by caller
 */
void mutil_real_escape_string(char *buf, char *val, size_t len);
/*
 * caller just need free() the *to;
 */
char* mutil_real_escape_string_nalloc(char **to, char *from, size_t len);

#ifndef DROP_FCGI
int read_cb(void *ptr, char *data, int size);
int printf_cb(void *ptr, const char *format, va_list ap);
int write_cb(void *ptr, const char *data, int size);
#endif

__END_DECLS
#endif    /* __MUTIL_H__ */
