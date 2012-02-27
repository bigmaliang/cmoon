#ifndef __MFILE_H__
#define __MFILE_H__

#include "mheads.h"

__BEGIN_DECLS

NEOERR* mfile_makesure_dir(char *file);
NEOERR* mfile_openf(FILE **fp, const char *mode, char *fname, ...)
                    ATTRIBUTE_PRINTF(3, 4);
NEOERR* mfile_copy(FILE *dst, FILE *src);

/*
 * fastcgi redefined FILE*, but cgi_filehandle() don't know,
 * so, cgi_filehandle() return's a standard FILE*
 * adn fseek, fread... need FCGI_FILE* as parameter in fastcgi mode
 * so, create a FCGI_FILE* from cgi_filehandle()'s FILE*.
 * ****************************************************************
 * you need to take care of every 3rd part library's file operation
 * e.g. gdImageXxx()
 *
 * you don't need to take care of fopen(), fseek()... on stdio.h
 * because of fcgi_stdio.h do it for you
 *
 * cgi_wrap_writef() can replace by printf() of mjson.c on fastcgi
 * we use cgi_wrap_writef() is for clearsilver's conveninece
 * ****************************************************************
 */
FILE* mfile_get_safe_from_std(FILE *in);
FILE* mfile_get_std_from_safe(FILE *in);

char* mfile_get_type(CGI *cgi, char *form_name);

__END_DECLS
#endif    /* __MFILE_H__ */
