#ifndef __MIMG_H__
#define __MIMG_H__

#include "mheads.h"

__BEGIN_DECLS

NEOERR* mimg_create_from_string(char *s, char *path, double size, void **pic);
NEOERR* mimg_zoomout(FILE *dst, FILE*src, int width, int height);
NEOERR* mimg_accept(CGI *cgi, char *imgroot, char result[LEN_MD5]);
NEOERR* mimg_accept_and_zoomout(CGI *cgi, char *imgroot, char result[LEN_MD5],
                                int width, int height);
NEOERR* mimg_output(void *pic);
//FIBITMAP* mimg_scale();

__END_DECLS
#endif    /* __MIMG_H__ */
