#include "mheads.h"

NEOERR* mfile_makesure_dir(char *file)
{
    if (file == NULL) return STATUS_OK;

    char tok[_POSIX_PATH_MAX];
    char *p = strchr(file, '/');

    while (p != NULL) {
        memset(tok, 0x0, sizeof(tok));
        strncpy(tok, file, p-file+1);
        if (mkdir(tok, 0755) != 0 && errno != EEXIST && errno != EISDIR) {
            return nerr_raise(NERR_IO, "mkdir %s failure %d %s",
                              tok, errno, strerror(errno));
        }
        while (*p == '/') p++;
        if(p) p = strchr(p, '/');
    }
    mtc_noise("directory %s ok", tok);
    return STATUS_OK;
}

NEOERR* mfile_openf(FILE **fp, const char *mode, char *fmt, ...)
{
	NEOERR *err;
    
    MCS_NOT_NULLC(fp, mode, fmt);

    char fname[LEN_FN];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(fname, sizeof(fname), fmt, ap);
    va_end(ap);
    
    err = mfile_makesure_dir(fname);
    if (err != STATUS_OK) return nerr_pass(err);

    FILE *fpout = fopen(fname, mode);
    if (!fpout) return nerr_raise(NERR_SYSTEM, "create %s failre", fname);

    *fp = fpout;

    return STATUS_OK;
}

NEOERR* mfile_copy(FILE *dst, FILE *src)
{
    char buf[1048576];
    size_t len;
    
    MCS_NOT_NULLB(dst, src);

    fseek(dst, 0, SEEK_SET);
    fseek(src, 0, SEEK_SET);

    while ((len = fread(buf, 1, sizeof(buf), src)) > 0) {
        fwrite(buf, 1, len, dst);
    }

    return STATUS_OK;
}

FILE* mfile_get_safe_from_std(FILE *in)
{
    if (!in) return NULL;

    FILE *fp;
    
#ifdef USE_FASTCGI
    fp = malloc(sizeof(FILE));
    if (fp) {
        fp->stdio_stream = in;
        fp->fcgx_stream = NULL;
    }
#else
    fp = in;
#endif

    return fp;
}

FILE* mfile_get_std_from_safe(FILE *in)
{
    if (!in) return NULL;

#ifdef USE_FASTCGI
    return (FILE*)in->stdio_stream;
#else
    return in;
#endif
}

/* TODO read file's type */
char* mfile_get_type(CGI *cgi, char *form_name)
{
    if (form_name) return hdf_get_valuef(cgi->hdf, PRE_QUERY".%s.Type", form_name);
    else return hdf_get_value(cgi->hdf, "HTTP.XFileType", NULL);
}
