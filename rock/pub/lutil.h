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
int CGI_REQ_TYPE(CGI *cgi);

void* lutil_get_data_handler(void *lib, CGI *cgi);
int lutil_fill_layout_by_file(mdb_conn *conn, file_t *file, HDF *hdf);
int lutil_image_accept(FILE *fp, char *path, unsigned char *result);
/*
 * multi image: prefix != NULL and name != NULL, e.g. Output.atoms, img
 * single image: prefix == NULL and name != NULL, e.g. Output.atom.img
 */
int lutil_image_expand(HDF *hdf, char *prefix, char *name,
                       char *imgpath, char *imgsize, char *dst);
int lutil_fetch_count(HDF *hdf, mdb_conn *conn, char *table, char *cond);
int lutil_fetchcountf(HDF *hdf, mdb_conn *conn, char *table, char *cfmt, ...);

__END_DECLS
#endif	/* __LUTIL_H__ */
