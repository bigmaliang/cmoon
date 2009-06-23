#ifndef __MMISC_H__
#define __MMISC_H__

#include "mheads.h"

__BEGIN_DECLS

#define QR_NUM_MAX	25

void exiting(void);
bool mmisc_getdatetime(char *res, int len, const char *fmt, time_t second);
int  mmisc_compare_int(const void *a, const void *b);
int  mmisc_compare_inta(const void *a, const void *b);

void mmisc_set_qrarray(char *qrcol, char qr_array[QR_NUM_MAX][LEN_ST], int *qr_cnt);

int mmisc_get_count(mdb_conn *conn, char *table, char *col);
void mmisc_set_count(HDF *hdf, mdb_conn *conn, char *table, char *col);
void mmisc_get_offset(HDF *hdf, int *count, int *offset);

/*
 * IE's bug: second must > 7200 
 */
void mmisc_cache_headers(time_t second);

__END_DECLS
#endif	/* __MMISC_H__ */
