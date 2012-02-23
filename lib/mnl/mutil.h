#ifndef __MUTIL_H__
#define __MUTIL_H__

#include "mheads.h"

__BEGIN_DECLS

#define SAFE_FREE(str)                          \
    do {                                        \
        if (str != NULL)                        \
            free(str);                          \
    } while (0)

bool    mutil_client_attack(HDF *hdf, char *action, char *cname,
                            uint64_t limit, time_t exp);
bool    mutil_client_attack_cookie(HDF *hdf, char *action,
                                   uint64_t limit, time_t exp);
void    mutil_makesure_coredump();

NEOERR* mutil_makesure_dir(char *file);
NEOERR* mutil_file_openf(FILE **fp, const char *mode, char *fname, ...);
NEOERR* mutil_file_copy(FILE *dst, FILE *src);

/*
 * res must be char xxx[LEN_TM]
 */
bool mutil_getdatetime(char *res, int len, const char *fmt, time_t second);
/*
 * res must be char xxx[LEN_TM_GMT]
 */
bool mutil_getdatetime_gmt(char *res, int len, const char *fmt, time_t second);

int  mutil_compare_int(const void *a, const void *b);
int  mutil_compare_inta(const void *a, const void *b);

__END_DECLS
#endif    /* __MUTIL_H__ */
