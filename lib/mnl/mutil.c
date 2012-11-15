#include "mheads.h"

bool mutil_client_attack(HDF *hdf, char *action, char *cname, uint64_t limit, time_t exp)
{
    uint64_t cntcn, cntip; cntcn = cntip = 0;
    char *cn = hdf_get_valuef(hdf, "Cookie.%s", cname);    cn = cn ? cn: "";
    char *ip = hdf_get_value(hdf, "CGI.RemoteAddress", "unknownHost");
    mmc_countf(MMC_OP_INC, 1, &cntcn, exp, 0, "%s.%s.%s", PRE_MMC_CLIENT, action, cn);
    mmc_countf(MMC_OP_INC, 1, &cntip, exp, 0, "%s.%s.%s", PRE_MMC_CLIENT, action, ip);
    if (cntcn >= limit || cntip >= limit) {
        hdf_set_int_value(hdf, PRE_OUTPUT".tired", cntcn);
        hdf_set_int_value(hdf, PRE_OUTPUT".limit", limit);
        hdf_set_int_value(hdf, PRE_OUTPUT".during", exp);
        return true;
    }
    return false;
}

bool mutil_client_attack_cookie(HDF *hdf, char *action, uint64_t limit, time_t exp)
{
    char scnt[LEN_MMC_KEY], sdur[LEN_MMC_KEY], val[64], tm[LEN_TM_GMT];
    snprintf(scnt, sizeof(scnt), "%s_%s_cnt", PRE_MMC_CLIENT, action);
    snprintf(sdur, sizeof(sdur), "%s_%s_dur", PRE_MMC_CLIENT, action);
    
    char tok[LEN_MMC_KEY];
    snprintf(tok, sizeof(tok), PRE_COOKIE".%s", scnt);
    int cnt = hdf_get_int_value(hdf, tok, 0);
    if (cnt > limit) {
        return true;
    }

    /*
     * not attack, store increment 
     */
    if (cnt < 0) cnt = 0;
    sprintf(val, "%d", cnt+1);
    snprintf(tok, sizeof(tok), PRE_COOKIE".%s", sdur);
    char *dur = hdf_get_value(hdf, tok, NULL);
    if (dur == NULL) {
        mutil_getdatetime_gmt(tm, sizeof(tm), "%A, %d-%b-%Y %T GMT", exp);
        dur = tm;
        cgi_cookie_set(NULL, sdur, dur, NULL, NULL, dur, 1, 0);
    }

    cgi_cookie_set(NULL, scnt, val, NULL, NULL, dur, 1, 0);
    return false;
}

void mutil_makesure_coredump()
{
    struct rlimit rl;

    rl.rlim_cur = 500*1024*1024;
    rl.rlim_max = 500*1024*1024;
    setrlimit(RLIMIT_CORE, &rl);
}

int mutil_systemf(char *fmt, ...)
{
    char key[LEN_HASH_KEY];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return system(key);
}

int mutil_execvf(char *argv[], char *fmt, ...)
{
    char key[LEN_HASH_KEY];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(key, sizeof(key), fmt, ap);
    va_end(ap);

    return execv(key, argv);
}

bool mutil_getdatetime(char *res, int len, const char *fmt, time_t second)
{
    memset(res, 0x0, len);
    time_t tm = time(NULL) + second;
    /* TODO memory leak? */
    struct tm *stm = localtime(&tm);
    if (strftime(res, len, fmt, stm) == 0)
        return false;
    return true;
}

bool mutil_getdatetime_gmt(char *res, int len, const char *fmt, time_t second)
{
    memset(res, 0x0, len);
    time_t tm = time(NULL) + second;
      struct tm *stm = gmtime(&tm);
    if (strftime(res, len, fmt, stm) == 0)
        return false;
    return true;
}

time_t mutil_get_abssec(char *fmt, char *time)
{
    char datetime[LEN_TM] = {0};
    struct tm thatdaystm;
    
    if (!fmt || !time) return 0;

    snprintf(datetime, LEN_TM, "%s", time);
    //strptime(datetime, "%Y-%m-%d %H:%M:%S", &thatdaystm);
    strptime(datetime, fmt, &thatdaystm);

    return mktime(&thatdaystm);
}

void mutil_utc_time(struct timespec *ts)
{
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, ts);
#endif    
}

int mutil_compare_int(const void *a, const void *b)
{
    int *i = (int*)a;
    int *j = (int*)b;
    return *i-*j;
}

int mutil_compare_inta(const void *a, const void *b)
{
    int *i = (int*)a;
    char *j = (char*)b;
    
    return *i - atoi(j);
}
