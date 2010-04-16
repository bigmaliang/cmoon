#include <stdlib.h>		/* malloc() */
#include <stdint.h>		/* uint32_t and friends */
#include <stdbool.h>		/* bool */
#include <string.h>		/* memcpy() */
#include <arpa/inet.h>		/* htonl() and friends */

#include "net-const.h"
#include "data.h"
#include "netutils.h"
#include "ClearSilver.h"


struct data_cell* data_cell_create_u32(const unsigned char *key, size_t ksize, uint32_t val)
{
	struct data_cell *c = calloc(1, sizeof(struct data_cell));
	if (c == NULL) return NULL;

	c->type = DATA_TYPE_U32;
	c->ksize = ksize+1;
	c->key = malloc(ksize+1);
	if (c->key != NULL) {
		memcpy(c->key, key, ksize);
		*(c->key+ksize) = '\0';
	}
	c->v.ival = val;
	return c;
}

struct data_cell* data_cell_create_ulong(const unsigned char *key, size_t ksize, unsigned long val)
{
	struct data_cell *c = calloc(1, sizeof(struct data_cell));
	if (c == NULL) return NULL;

	c->type = DATA_TYPE_ULONG;
	c->ksize = ksize+1;
	c->key = malloc(ksize+1);
	if (c->key != NULL) {
		memcpy(c->key, key, ksize);
		*(c->key+ksize) = '\0';
	}
	c->v.lval = val;
	return c;
}

struct data_cell* data_cell_create_str(const unsigned char *key, size_t ksize,
									   const unsigned char *val, size_t vsize)
{
	struct data_cell *c = calloc(1, sizeof(struct data_cell));
	if (c == NULL) return NULL;

	c->type = DATA_TYPE_STRING;
	c->ksize = ksize+1;
	c->key = malloc(ksize+1);
	if (c->key != NULL) {
		memcpy(c->key, key, ksize);
		*(c->key+ksize) = '\0';
	}
	c->v.sval.vsize = vsize+1;
	c->v.sval.val = malloc(vsize+1);
	if (c->v.sval.val != NULL) {
		if (vsize > 0) memcpy(c->v.sval.val, val, vsize);
		*(c->v.sval.val+vsize) = '\0';
	}
	return c;
}

struct data_cell* data_cell_create_array(const unsigned char *key, size_t ksize,
										 size_t vcount, unsigned char *buf, uint32_t *vsize)
{
	struct data_cell *c = calloc(1, sizeof(struct data_cell));
	if (c == NULL) return NULL;

	c->type = DATA_TYPE_ARRAY;
	c->ksize = ksize+1;
	c->key = malloc(ksize+1);
	if (c->key != NULL) {
		memcpy(c->key, key, ksize);
		*(c->key+ksize) = '\0';
	}
	uListInit(&c->v.aval, 0, 0);

	const unsigned char *mykey, *val;
	uint32_t myksize, vtype, mysize, mycount, ival;
	unsigned long lval;
	unsigned char *pos = buf;
	struct data_cell *cell;
	size_t i;
	for (i = 0; i < vcount; i++) {
		vtype = * (uint32_t *) pos; vtype = ntohl(vtype);

		pos = pos + sizeof(uint32_t);
		myksize = * (uint32_t *) pos; myksize = ntohl(myksize);

		pos = pos + sizeof(uint32_t);
		mykey = pos;

		pos = pos + myksize;
		
		if (vtype == DATA_TYPE_U32) {
			ival = * (uint32_t *) pos; ival = ntohl(ival);
			pos = pos + sizeof(uint32_t);
			cell = data_cell_create_u32(mykey, myksize, ival);
		} else if (vtype == DATA_TYPE_ULONG) {
			lval = * (uint64_t *)pos; lval = ntohll(lval);
			pos = pos + sizeof(uint64_t);
			cell = data_cell_create_ulong(mykey, myksize, lval);
		} else if (vtype == DATA_TYPE_STRING){
			mysize = * (uint32_t *) pos; mysize = ntohl(mysize);
			pos = pos + sizeof(uint32_t);
			val = pos;
			pos = pos + mysize;
			cell = data_cell_create_str(mykey, myksize, val, mysize);
		} else if (vtype == DATA_TYPE_ARRAY) {
			mycount = * (uint32_t *) pos; mycount = ntohl(mycount);
			pos = pos + sizeof(uint32_t);
			cell = data_cell_create_array(mykey, myksize, mycount, pos, &mysize);
			pos = pos + mysize;
		} else
			continue;
		if (cell != NULL) {
			uListAppend(c->v.aval, (void*)cell);
		}
	}
	*vsize = (size_t) (pos - buf);
	
	uListSort(c->v.aval, data_cell_compare);
	return c;
}

void data_cell_append(struct data_cell *pc, struct data_cell *c)
{
	if (pc == NULL || c == NULL || pc->type != DATA_TYPE_ARRAY) return;

	struct data_cell *oldc = data_cell_search(pc, false, c->type,
											  (const char*)c->key);
	if (oldc != NULL) {
		data_cell_replace_val(oldc, c);
		/* TODO free new array */
		if (c->type != DATA_TYPE_ARRAY)
			data_cell_free(c);
	} else {
		uListAppend(pc->v.aval, (void*)c);
		uListSort(pc->v.aval, data_cell_compare);
	}
}

int data_cell_replace_val(struct data_cell *dst, struct data_cell *src)
{
	if (dst == NULL || src == NULL) return -1;

	if (dst->type == DATA_TYPE_U32 && src->type == DATA_TYPE_U32) {
		dst->v.ival = src->v.ival;
	} else if (dst->type == DATA_TYPE_ULONG && src->type == DATA_TYPE_ULONG) {
		dst->v.lval = src->v.lval;
	} else if (dst->type == DATA_TYPE_STRING && src->type == DATA_TYPE_STRING) {
		if (dst->v.sval.val != NULL) free(dst->v.sval.val);
		dst->v.sval.val = (unsigned char*)calloc(1, src->v.sval.vsize);
		if (dst->v.sval.val == NULL) return -2;
		memcpy(dst->v.sval.val, src->v.sval.val, src->v.sval.vsize);
		dst->v.sval.vsize = src->v.sval.vsize;
	} else if (dst->type == DATA_TYPE_ARRAY && src->type == DATA_TYPE_ARRAY) {
		uListDestroyFunc(&dst->v.aval, data_cell_free);
		dst->v.aval = src->v.aval;
	} else
		return -3;

	return 1;
}

static int depth = 0;
void data_cell_dump(struct data_cell *c)
{
	if (c == NULL) return;

	int len, i, ti;
	struct data_cell *lc;

	depth++;
	
#define DEPTHP													\
	do {for (ti = 1; ti < depth; ti++) printf("\t");} while(0)
	
	DEPTHP; printf("%s", c->key);
	switch (c->type) {
	case DATA_TYPE_U32:
		printf("(i):  %d\n", c->v.ival);
		break;
	case DATA_TYPE_ULONG:
		printf("(l): %lu\n", c->v.lval);
		break;
	case DATA_TYPE_STRING:
		printf("(s):  %s\n", c->v.sval.val);
		break;
	case DATA_TYPE_ARRAY:
		printf("\n");
		len = uListLength(c->v.aval);
		for (i = 0; i < len; i++) {
			uListGet(c->v.aval, i, (void**)&lc);
			data_cell_dump(lc);
		}
		break;
	default:
		DEPTHP;	printf("unkown type cell");
		break;
	}

	depth--;
}

size_t data_cell_size(struct data_cell *c)
{
	if (c == NULL) return 0;
	
	size_t rv = sizeof(struct data_cell);
	rv += c->ksize;
	struct data_cell *cc;
	
	switch (c->type) {
	case DATA_TYPE_U32:
	case DATA_TYPE_ULONG:
		break;
	case DATA_TYPE_STRING:
		rv += c->v.sval.vsize;
		break;
	case DATA_TYPE_ARRAY:
		data_cell_array_iterate(c, cc) {
			rv += data_cell_size(cc);
		}
		break;
	}
	
	return rv;
}

struct data_cell* data_cell_alloc_array(const char *key)
{
	if (key == NULL) return NULL;
	size_t ksize = strlen(key);
	
	struct data_cell *c = calloc(1, sizeof(struct data_cell));
	if (c == NULL) return NULL;

	c->type = DATA_TYPE_ARRAY;
	c->ksize = ksize+1;
	c->key = malloc(ksize+1);
	if (c->key != NULL) {
		memcpy(c->key, key, ksize);
		*(c->key+ksize) = '\0';
	}
	uListInit(&c->v.aval, 0, 0);

	return c;
}

void data_cell_free(void *cell)
{
	struct data_cell *c = (struct data_cell*)cell;
	
	if (c == NULL)
		return;
	if (c->key != NULL) free(c->key);
	if (c->type == DATA_TYPE_STRING) {
		if (c->v.sval.val != NULL)
			free(c->v.sval.val);
	} else if (c->type == DATA_TYPE_ARRAY) {
		uListDestroyFunc(&c->v.aval, data_cell_free);
	}
	free(c);
}

int data_cell_compare(const void *pa, const void *pb)
{
	struct data_cell *a, *b;
	a = *(struct data_cell**)pa;
	b = *(struct data_cell**)pb;

	size_t max = a->ksize > b->ksize? a->ksize: b->ksize;
	
	if (a->type == DATA_TYPE_ANY) {
		return strncmp((char*)a->key, (char*)b->key, max);
	} else if (a->type != b->type) {
		return (int)(a->type - b->type);
	} else {
		return strncmp((char*)a->key, (char*)b->key, max);
	}
}

struct data_cell* data_cell_search(struct data_cell *dataset, bool recursive,
								   unsigned int type, const char *key)
{
	struct data_cell *res;
	if (dataset == NULL || key == NULL) return NULL;

	if (dataset->type != DATA_TYPE_ARRAY ) {
		if (dataset->type == type &&
			strncmp((const char*)dataset->key, key, strlen(key)) == 0)
			return dataset;
		res = NULL;
		goto done;
	}
	
	struct data_cell *c = calloc(1, sizeof(struct data_cell));
	if (c == NULL) return NULL;
	
	c->type = type;
	c->key = (unsigned char*)strdup(key);
	c->ksize = strlen(key);

	res = (struct data_cell*)uListSearch(dataset->v.aval,
										 (const void*)&c, data_cell_compare);
	data_cell_free(c);

 done:
	if (res == NULL) {
		if (recursive) {
			struct data_cell *lc;
			NEOERR *err;
			int len = uListLength(dataset->v.aval);
			int i;
			for (i = 0; i < len; i++) {
				err = uListGet(dataset->v.aval, i, (void**)&lc);
				if (err != STATUS_OK) continue;
				if (lc->type == DATA_TYPE_ARRAY) {
					res = data_cell_search(lc, recursive, type, key);
					if (res != NULL) return res;
				}
			}
		}
		return NULL;
	} else
		return *(struct data_cell**)res;
}

struct data_cell* data_cell_add_u32(struct data_cell *dataset, const char *parent,
									const char *key, uint32_t val)
{
	if (dataset == NULL || dataset->type != DATA_TYPE_ARRAY) return NULL;
	
	struct data_cell *pc = data_cell_search(dataset, true, DATA_TYPE_ARRAY, parent);
	if (pc == NULL && (parent == NULL || !strcmp(parent, ""))) pc = dataset;
	if (pc == NULL) return NULL;

	struct data_cell *c = data_cell_create_u32((const unsigned char*)key, strlen(key), val);
	if (c == NULL) return NULL;

	data_cell_append(pc, c);
	
	return c;
}

struct data_cell* data_cell_add_ulong(struct data_cell *dataset, const char *parent,
									  const char *key, unsigned long val)
{
	if (dataset == NULL || dataset->type != DATA_TYPE_ARRAY) return NULL;
	
	struct data_cell *pc = data_cell_search(dataset, true, DATA_TYPE_ARRAY, parent);
	if (pc == NULL && (parent == NULL || !strcmp(parent, ""))) pc = dataset;
	if (pc == NULL) return NULL;

	struct data_cell *c = data_cell_create_ulong((const unsigned char*)key, strlen(key), val);
	if (c == NULL) return NULL;
	
	data_cell_append(pc, c);
	
	return c;
}

struct data_cell* data_cell_add_str(struct data_cell *dataset, const char *parent,
									const char *key, const char *val)
{
	if (dataset == NULL || dataset->type != DATA_TYPE_ARRAY ||
		key == NULL || val == NULL) return NULL;
	
	struct data_cell *pc = data_cell_search(dataset, true, DATA_TYPE_ARRAY, parent);
	if (pc == NULL && (parent == NULL || !strcmp(parent, ""))) pc = dataset;
	if (pc == NULL) return NULL;

	struct data_cell *c = data_cell_create_str((const unsigned char*)key, strlen(key),
											   (const unsigned char*)val, strlen(val));
	if (c == NULL) return NULL;
	
	data_cell_append(pc, c);
	
	return c;
}

struct data_cell* data_cell_add_array(struct data_cell *dataset, const char *parent,
									  const char *key)
{
	if (dataset == NULL || dataset->type != DATA_TYPE_ARRAY) return NULL;
	
	struct data_cell *pc = data_cell_search(dataset, true, DATA_TYPE_ARRAY, parent);
	if (pc == NULL && (parent == NULL || !strcmp(parent, ""))) pc = dataset;
	if (pc == NULL) return NULL;

	struct data_cell *c = data_cell_alloc_array(key);
	if (c == NULL) return NULL;

	data_cell_append(pc, c);
	
	return c;
}


int data_cell_length(struct data_cell *c)
{
	if (c == NULL) return 0;
	
	if (c->type != DATA_TYPE_ARRAY) return 1;

	return uListLength(c->v.aval);
}
struct data_cell* data_cell_get(struct data_cell *c, int i)
{
	struct data_cell *rc;
	NEOERR *err;
	
	if (c == NULL || c->type != DATA_TYPE_ARRAY) return NULL;
	
	err = uListGet(c->v.aval, i, (void**)&rc);
	if (err == STATUS_OK) return rc;
	return NULL;
}
