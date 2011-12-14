#ifndef __MHTTP_H__
#define __MHTTP_H__

#include "mheads.h"

__BEGIN_DECLS

/*
 * With PUT and DELETE, there is a big difference between zero request and one,
 * but there is no difference between one request and ten.
 */
enum {
    CGI_REQ_GET = 0,
    CGI_REQ_PUT,
    CGI_REQ_POST,
    CGI_REQ_DEL,
    CGI_REQ_UNKNOWN
};

int http_req_method(CGI *cgi);

/*
 * Cache-Control
 * IE: make sure timezone & time set correct on web server
 */
void mhttp_cache_headers(time_t second);

#ifdef USE_FASTCGI
int read_cb(void *ptr, char *data, int size);
int printf_cb(void *ptr, const char *format, va_list ap);
int write_cb(void *ptr, const char *data, int size);
#endif

__END_DECLS
#endif    /* __MHTTP_H__ */
