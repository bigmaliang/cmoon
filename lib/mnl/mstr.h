#ifndef __MSTR_H__
#define __MSTR_H__

#include "mheads.h"

__BEGIN_DECLS

void mstr_rand_string(char *s, int max);
void mstr_html_escape(HDF *hdf, char *name);
void mstr_html_unescape(HDF *hdf, char *name);
/*
 * escape <script...>...</script> to &lt;script...>....&lt;/script&gt;
 */
void mstr_script_escape(HDF *node, char *name);

/* out must be 33 length */
void mstr_md5_buf(unsigned char *in, size_t len, char out[LEN_MD5]);
void mstr_md5_str(char *in, char out[LEN_MD5]);
bool mstr_isdigit(char *s);
bool mstr_isdigitn(char *buf, size_t len);
bool mstr_israngen(char *buf, size_t len, int *left, int *right);

/*
 * make sure buf big enough please
 * both alloc() and free() MUST be done by caller
 */
void mstr_real_escape_string(char *buf, char *val, size_t len);
/*
 * caller just need free() the *to;
 */
char* mstr_real_escape_string_nalloc(char **to, char *from, size_t len);

void mstr_repchr(char *s, char from, char to);
/*
 * replace string with string
 * return an allocated string, remember to free it
 * mstr_repstr(s, "from1", "to1", "from2", "to2", ...)
 * make sure offer suitable rep_count, or, random errors will occur.
 */
char* mstr_repstr(int rep_count, char *s, ...);
char* mstr_strip (char *s, char n);
/* string's utf-8 length */
size_t mstr_ulen(const char *s);
/* string's strlen(), to positon pos */
long int mstr_upos2len(const char *s, long int pos);

/* DJB Hash (left to right, ....abc)*/
unsigned int hash_string(const char *str);
/* DJB Hash revert (right to left, abc... )*/
unsigned int hash_string_rev(const char *str);

/*
 * make sure charout has inlen*2+1 len
 */
void mstr_hex2str(unsigned char *hexin, unsigned int inlen, unsigned char *charout);
void mstr_bin2char(unsigned char *in, unsigned int inlen, unsigned char *out);

__END_DECLS
#endif    /* __MSTR_H__ */
