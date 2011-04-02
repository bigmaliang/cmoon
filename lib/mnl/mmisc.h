#ifndef __MMISC_H__
#define __MMISC_H__

#include "mheads.h"

__BEGIN_DECLS

#define SAFE_FREE(str)							\
	do {										\
		if (str != NULL)						\
			free(str);							\
	} while (0)

/* table, condition MUST be string literal, not variable */
#define MMISC_PAGEDIV_SET_N(hdf, db, table, condition, sfmt, ...)		\
	do {																\
		int zinta;														\
		mdb_exec(db, NULL, "SELECT COUNT(*) FROM " table " WHERE " condition ";", sfmt, ##__VA_ARGS__); \
		mdb_get(db, "i", &zinta);										\
		hdf_set_int_value(hdf, "ntt", zinta);							\
	} while (0)
#define MMISC_PAGEDIV_SET(hdf, outprefix, db, table, condition, sfmt, ...) \
	do {																\
		int zinta;														\
		mdb_exec(db, NULL, "SELECT COUNT(*) FROM " table " WHERE " condition ";", sfmt, ##__VA_ARGS__); \
		mdb_get(db, "i", &zinta);										\
		hdf_set_valuef(hdf, "%s.ntt=%d", outprefix, zinta);				\
	} while (0)


void exiting(void);
bool mmisc_getdatetime(char *res, int len, const char *fmt, time_t second);
bool mmisc_getdatetime_gmt(char *res, int len, const char *fmt, time_t second);
int  mmisc_compare_int(const void *a, const void *b);
int  mmisc_compare_inta(const void *a, const void *b);

void mmisc_set_qrarray(char *qrcol, char qr_array[QR_NUM_MAX][LEN_ST], int *qr_cnt);

void mmisc_pagediv(HDF *hdf, char *inprefix, int *count, int *offset,
				   char *outprefix, HDF *ohdf);
void mmisc_str_repchr(char *s, char from, char to);
/*
 * return an allocated string, remember to free it
 * mmisc_str_repchr(s, "from1", "to1", "from2", "to2", ...)
 * make sure offer suitable rep_count, or, random errors will occur.
 */
char* mmisc_str_repstr(int rep_count, char *s, ...);
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
void mmisc_bin2char(unsigned char *in, unsigned int inlen, unsigned char *out);

__END_DECLS
#endif	/* __MMISC_H__ */
