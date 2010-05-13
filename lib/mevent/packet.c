#include <stdlib.h>		/* malloc() */
#include <stdint.h>		/* uint32_t and friends */
#include <string.h>		/* memcpy() */
#include <arpa/inet.h>		/* htonl() and friends */

#include "net-const.h"
#include "data.h"
#include "netutils.h"
#include "packet.h"

size_t unpack_data(const char *key, unsigned char *buf, size_t len,
				   struct data_cell **dataset)
{
	const unsigned char *mykey, *val;
	uint32_t ksize, vtype, vsize, vcount, ival, ttsize;
	unsigned long lval;
	unsigned char *pos;
	struct data_cell *rcell, *cell, *oldcell;

	if (key == NULL || buf == NULL) return 0;

	pos = buf;
	vtype = * (uint32_t *) pos; vtype = ntohl(vtype);
	ttsize = sizeof(uint32_t);

	if (vtype == DATA_TYPE_EOF) return 0;

	rcell = data_cell_alloc_array(key);
	if (rcell == NULL) return 0;

	while (vtype != DATA_TYPE_EOF && ttsize < len) {
		pos = pos + sizeof(uint32_t);
		ksize = * (uint32_t *) pos; ksize = ntohl(ksize);

		pos = pos + sizeof(uint32_t);
		mykey = pos;

		pos = pos + ksize;
		ttsize += ksize + 2*sizeof(uint32_t);
		
		if (vtype == DATA_TYPE_U32) {
			ival = * (uint32_t *) pos; ival = ntohl(ival);
			pos = pos + sizeof(uint32_t);
			cell = data_cell_create_u32(mykey, ksize, ival);
			ttsize += sizeof(uint32_t);
		} else if (vtype == DATA_TYPE_ULONG) {
			lval = *(uint64_t *)pos; lval = ntohll(lval);
			pos = pos + sizeof(uint64_t);
			cell = data_cell_create_ulong(mykey, ksize, lval);
			ttsize += sizeof(uint64_t);
		} else if (vtype == DATA_TYPE_STRING){
			vsize = * (uint32_t *) pos; vsize = ntohl(vsize);
			pos = pos + sizeof(uint32_t);
			val = pos;
			pos = pos + vsize;
			cell = data_cell_create_str(mykey, ksize, val, vsize);
			ttsize += sizeof(uint32_t) + vsize;
		} else if (vtype == DATA_TYPE_ARRAY) {
			vcount = * (uint32_t *) pos; vcount = ntohl(vcount);
			pos = pos + sizeof(uint32_t);
			cell = data_cell_create_array(mykey, ksize, vcount, pos, &vsize);
			pos = pos + vsize;
			ttsize += sizeof(uint32_t) + vsize;
		} else
			continue;
		if (cell != NULL) {
			oldcell = data_cell_search(rcell, false, cell->type,
									   (const char*)cell->key);
			if (oldcell != NULL) {
				data_cell_replace_val(oldcell, cell);
				/* TODO free new array */
				if (cell->type != DATA_TYPE_ARRAY)
					data_cell_free(cell);
			} else {
				data_cell_append(rcell, (void*)cell);
			}
		}

		vtype = * (uint32_t *) pos; vtype = ntohl(vtype);
	}
	if (*dataset) data_cell_free(*dataset);
	*dataset = rcell;

	return ttsize;
}

size_t pack_data_u32(const char *key, uint32_t val, unsigned char *buf)
{
	size_t ksize = strlen(key);
	
	* (uint32_t *) buf = htonl(DATA_TYPE_U32);
	* ((uint32_t *) buf + 1) = htonl(ksize);
	memcpy(buf+8, key, ksize);
	* ((uint32_t *) (buf + 8 + ksize)) = htonl(val);

	return (8 + ksize + sizeof(uint32_t));
}

size_t pack_data_ulong(const char *key, unsigned long val, unsigned char *buf)
{
	size_t ksize = strlen(key);

	* (uint32_t *)buf = htonl(DATA_TYPE_ULONG);
	* ((uint32_t *) buf + 1) = htonl(ksize);
	memcpy(buf+8, key, ksize);
	* ((uint64_t *) (buf + 8 + ksize)) = htonll(val);

	return (8 + ksize + sizeof(uint64_t));
}

size_t pack_data_str(const char *key, const char *val, unsigned char *buf)
{
	size_t ksize = strlen(key);
	size_t vsize = 0;

    if (val) vsize = strlen(val);
	
	* (uint32_t *) buf = htonl(DATA_TYPE_STRING);
	* ((uint32_t *) buf + 1) = htonl(ksize);
	memcpy(buf+8, key, ksize);
	* ((uint32_t *) (buf + 8 + ksize)) = htonl(vsize);
    if (vsize > 0) memcpy(buf+8+ksize+4, val, vsize);

	return (8 + ksize + 4 + vsize);
}

size_t pack_data_array(const char *key, struct data_cell *dataset,
					   unsigned char *buf, size_t maxsize)
{
	struct data_cell *cell;
	size_t psize, ksize;
	int len, i;

	if (dataset == NULL || dataset->type != DATA_TYPE_ARRAY ||
		maxsize < RESERVE_SIZE) return 0;
	
	len = data_cell_length(dataset);

	psize = ksize = 0;

	if (key != NULL) {
		ksize = strlen(key);
		
		* (uint32_t *) buf = htonl(DATA_TYPE_ARRAY);
		* ((uint32_t *) buf + 1) = htonl(ksize);
		memcpy(buf+8, key, ksize);
		* ((uint32_t *) (buf + 8 + ksize)) = htonl(len);
		
		psize += 8 + ksize + 4;
	}
	
	for (i = 0; i < len; i++) {
		cell = data_cell_get(dataset, i);
		if (cell == NULL) continue;
		switch (cell->type) {
		case DATA_TYPE_U32:
			psize += pack_data_u32((const char*)cell->key,
								   cell->v.ival, buf+psize);
			break;
		case DATA_TYPE_ULONG:
			psize += pack_data_ulong((const char*)cell->key,
									 cell->v.lval, buf+psize);
			break;
		case DATA_TYPE_STRING:
			psize += pack_data_str((const char*)cell->key,
								   (const char*)cell->v.sval.val, buf+psize);
			break;
		case DATA_TYPE_ARRAY:
			psize += pack_data_array((const char*)cell->key,
									 cell, buf+psize, maxsize-psize);
			break;
		default:
			break;
		}
		if (psize + RESERVE_SIZE > maxsize) break;
	}

	if (ksize != 0 && i < len) {
		* ((uint32_t *) (buf + 8 + ksize)) = htonl(--i);
	}
	
	return psize;
}

size_t pack_data_any(const char *key, struct data_cell *dataset,
                     unsigned char *buf, size_t maxsize)
{
    struct data_cell *c = dataset;
    if (c == NULL || maxsize < RESERVE_SIZE) return 0;

    switch(dataset->type) {
    case DATA_TYPE_U32:
        return pack_data_u32((char*)c->key, c->v.ival, buf);
    case DATA_TYPE_ULONG:
        return pack_data_ulong((char*)c->key, c->v.lval, buf);
    case DATA_TYPE_STRING:
        return pack_data_str((char*)c->key, (char*)c->v.sval.val, buf);
    case DATA_TYPE_ARRAY:
        return pack_data_array(key, dataset, buf, maxsize);
    default:
        return 0;
    }
}
