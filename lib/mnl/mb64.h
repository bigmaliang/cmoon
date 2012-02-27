#ifndef __MB64_H__
#define __MB64_H__

#include "mheads.h"

__BEGIN_DECLS

/*
 * base64 coder from http://base64.sourceforge.net/b64.c
 */

/*
** encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
void mb64_encodeblock(unsigned char in[3], unsigned char out[4], int len);

/*
** encode
**
** base64 encode a stream adding padding and line breaks as per spec.
*/
NEOERR* mb64_encode(FILE *infile, FILE *outfile, int linesize);

/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
void mb64_decodeblock(unsigned char in[4], unsigned char out[3]);

/*
** decode
**
** decode a base64 encoded stream discarding padding, line breaks and noise
*/
NEOERR* mb64_decode(FILE *infile, FILE *outfile);

__END_DECLS
#endif    /* __MB64_H__ */
