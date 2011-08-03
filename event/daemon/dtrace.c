
#include <stdio.h>        /* vsprintf() */
#include <stdarg.h>
#include <sys/types.h>         /* open() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>         /* write() */
#include <string.h>        /* strcmp(), strerror() */
#include <errno.h>        /* errno */
#include <time.h>        /* time() and friends */

#include "dtrace.h"
#include "common.h"

static char *g_trace_level[TC_LEVELS] = {"DIE", "ERROR", "WARNING", "DEBUG", "INFO", "NOISE"};

#if 0
static void trace_shift_file(FILE *fp)
{
    struct stat fs;
    if (stat(g_fn, &fs) == -1)
        return;
    if (fs.st_size < TC_MAX_SIZE)
        return;

    int i;
    char ofn[LEN_FN], nfn[LEN_FN];

    if (fp != NULL)
        FCLOSE(fp);

    for (i = TC_MAX_NUM-1; i > 1; i--) {
        sprintf(ofn, "%s.%d", g_fn, i-1);
        sprintf(nfn, "%s.%d", g_fn, i);
        rename(ofn, nfn);
    }
    if (TC_MAX_NUM > 1) {
        strcpy(ofn, g_fn);
        sprintf(nfn, "%s.1", g_fn);
        rename(ofn, nfn);
    }

    fp = FOPEN(g_fn, "a+");
}
#endif

static bool dtc_getdatetime(char *res, int len, const char *fmt, time_t second)
{
    memset(res, 0x0, len);
    time_t tm = time(NULL) + second;
    struct tm stm;
    localtime_r(&tm, &stm);
    if (strftime(res, len, fmt, &stm) == 0)
        return false;
    return true;
}

FILE* dtc_init(const char *fn)
{
    char g_fn[256];
    FILE *fp;
    
    strncpy(g_fn, fn, sizeof(g_fn)-4);
    strcat(g_fn, ".log");
    fp = FOPEN(g_fn, "a+");
    if (fp != NULL)    setvbuf(fp, (char *)NULL, _IOLBF, 0);
    return fp;
}
void dtc_leave(FILE *fp)
{
    if (fp != NULL)
        FCLOSE(fp);
    fp = NULL;
}

bool dtc_msg(FILE *fp, const char *func, const char *file, long line,
         int level, const char *format, ...)
{
    int dftlv = settings.trace_level;
    if (dftlv < 0 || dftlv > TC_LEVELS)
        dftlv = TC_WARNING;
    if (level > dftlv)
        return true;
    
    va_list ap;
    char tm[25];
    if (!dtc_getdatetime(tm, sizeof(tm), "%F %T", 0))
        return false;

    if (fp == NULL)
        return false;
    FPRINTF(fp, "[%s]", tm);
    FPRINTF(fp, "[%s]", g_trace_level[level]);
    FPRINTF(fp, "[%s:%li %s] ", file, line, func);

    va_start(ap, (void*)format);
    VFPRINTF(fp, format, ap);
    va_end(ap);

    FPRINTF(fp, "\n");
    //FFLUSH(NULL);
    //FSYNC(fileno(fp));

    //trace_shift_file();
    return true;
}
