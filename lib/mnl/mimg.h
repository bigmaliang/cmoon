#ifndef __MIMG_H__
#define __MIMG_H__

#include "mheads.h"

__BEGIN_DECLS

enum {
    MIMG_TYPE_JPEG = 0,
    MIMG_TYPE_PNG,
    MIMG_TYPE_GIF,
    MIMG_TYPE_BMP,
    MIMG_TYPE_UNKNOWN
};

int mimg_type_str2int(char *type);
char* mimg_type_int2str(int type);

NEOERR* mimg_create_from_string(char *s, char *path, double size, void **pic);
NEOERR* mimg_accept(CGI *cgi, char *form_name, char *imgroot,
                    char result[LEN_MD5], int *ftype);
NEOERR* mimg_zoomout(int ftype, FILE *dst, FILE*src, int width, int height);
NEOERR* mimg_accept_and_zoomout(CGI *cgi, char *form_name, char *imgroot,
                                char result[LEN_MD5],
                                int *ftype, int width, int height);
NEOERR* mimg_output(void *pic);

__END_DECLS
#endif    /* __MIMG_H__ */
