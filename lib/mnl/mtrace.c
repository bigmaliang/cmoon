#include "mtrace.h"

/* global file name for trace info write to */
static char g_fn[_POSIX_PATH_MAX] = "";
static FILE *g_fp = NULL;
static int m_dftlv = TC_DEFAULT_LEVEL;
static char linebuf[2096];
static char *g_trace_level[TC_LEVELS] = {"DIE", "MESSAGE", "ERROR", "WARNING", "INFO", "DEBUG", "NOISE"};

static void trace_shift_file()
{
    struct stat fs;
    if (stat(g_fn, &fs) == -1)
        return;
    if (fs.st_size < TC_MAX_SIZE)
        return;

    int i;
    char ofn[_POSIX_PATH_MAX], nfn[_POSIX_PATH_MAX];

    if (g_fp != NULL)
        fclose(g_fp);

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

    g_fp = fopen(g_fn, "a+");
}

void mtc_init(const char *fn, int level)
{
    if (level > 0 && level <= TC_LEVELS) m_dftlv = level;
    
    strncpy(g_fn, fn, sizeof(g_fn)-4);
    strcat(g_fn, ".log");
    if (g_fp != NULL)
        fclose(g_fp);
    g_fp = fopen(g_fn, "a+");
    if (g_fp != NULL) setvbuf(g_fp, linebuf, _IOLBF, 2096);
    atexit(mtc_leave);
}
void mtc_leave()
{
    if (g_fp != NULL)
        fclose(g_fp);
    g_fp = NULL;
    memset(g_fn, 0x0, sizeof(g_fn));
}

bool mtc_msg(const char *func, const char *file, long line,
             int level, const char *format, ...)
{
    //int dftlv = hdf_get_int_value(g_cfg, PRE_CONFIG".trace_level", TC_DEFAULT_LEVEL);
    if (level > m_dftlv) return false;
    
    if (g_fp == NULL) return false;

    va_list ap;
    char tm[25] = {0};
    time_t sec = time(NULL);
    //mutil_getdatetime(tm, sizeof(tm), "%Y-%m-%d %H:%M:%S", time(NULL));
    struct tm *stm = localtime(&sec);
    strftime(tm, 25, "%Y-%m-%d %H:%M:%S", stm);
    tm[24] = '\0';

    fprintf(g_fp, "[%s]", tm);
    fprintf(g_fp, "[%s]", g_trace_level[level]);
    fprintf(g_fp, "[%s:%li %s] ", file, line, func);

    va_start(ap, (void*)format);
    vfprintf(g_fp, format, ap);
    va_end(ap);

    fprintf(g_fp, "\n");

    trace_shift_file();
    return true;
}
