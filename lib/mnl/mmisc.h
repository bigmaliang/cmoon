#ifndef __MMISC_H__
#define __MMISC_H__

#include "mheads.h"

__BEGIN_DECLS

#define QR_NUM_MAX	25

void exiting(void);
bool mmisc_getdatetime(char *res, int len, const char *fmt, time_t second);
bool mmisc_getdatetime_gmt(char *res, int len, const char *fmt, time_t second);
int  mmisc_compare_int(const void *a, const void *b);
int  mmisc_compare_inta(const void *a, const void *b);

void mmisc_set_qrarray(char *qrcol, char qr_array[QR_NUM_MAX][LEN_ST], int *qr_cnt);

int mmisc_get_count(mdb_conn *conn, char *table, char *col);
void mmisc_set_count(HDF *hdf, mdb_conn *conn, char *table, char *col);
void mmisc_get_offset(HDF *hdf, int *count, int *offset);
void mmisc_str_repchr(char *s, char from, char to);
char* mmisc_str_strip (char *s, char n);
unsigned int hash_string(const char *str);

/*
 * IE: make sure timezone & time set correct on web server
 */
void mmisc_cache_headers(time_t second);

/*
 * make sure charout has inlen*2+1 len
 */
void mmisc_hex2str(unsigned char *hexin, unsigned int inlen, unsigned char *charout);

__END_DECLS
#endif	/* __MMISC_H__ */
