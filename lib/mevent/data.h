
#ifndef __MVTDATA_H__
#define __MVTDATA_H__

#include <stdint.h>		/* for uint32_t */
#include <stdbool.h>		/* bool */
#include "ClearSilver.h"

struct data_cell {
	unsigned int type;
	/*
	 * ksize == strlen(key) 
	 */
	uint32_t ksize;
	/*
	 * *(key+ksize) = '\0'
	 */
	unsigned char *key;
	union {
		uint32_t ival;
		unsigned long lval;
		struct {
			/* same as ksize and key */
			uint32_t vsize;
			unsigned char *val;
		} sval;
		ULIST *aval;
	} v;
};

struct data_cell* data_cell_create_u32(const unsigned char *key, size_t ksize,
									   uint32_t val);
struct data_cell* data_cell_create_ulong(const unsigned char *key, size_t ksize,
										 unsigned long val);
struct data_cell* data_cell_create_str(const unsigned char *key, size_t ksize,
									   const unsigned char *val, size_t vsize);
struct data_cell* data_cell_create_array(const unsigned char *key, size_t ksize,
										 size_t vcount, unsigned char *buf,
										 uint32_t *vsize);
void data_cell_append(struct data_cell *pc, struct data_cell *c);
int data_cell_replace_val(struct data_cell *dst, struct data_cell *src);
void data_cell_dump(struct data_cell *c);
size_t data_cell_size(struct data_cell *c);
struct data_cell* data_cell_alloc_array(const char *key);
void data_cell_free(void *cell);
int data_cell_compare(const void *pa, const void *pb);
struct data_cell* data_cell_search(struct data_cell *dataset, bool recursive,
								   unsigned int type, const char *key);

struct data_cell* data_cell_add_u32(struct data_cell *dataset, const char *parent,
									const char *key, uint32_t val);
struct data_cell* data_cell_add_ulong(struct data_cell *dataset, const char *parent,
									  const char *key, unsigned long val);
struct data_cell* data_cell_add_str(struct data_cell *dataset, const char *parent,
									const char *key, const char *val);
struct data_cell* data_cell_add_array(struct data_cell *dataset, const char *parent,
									  const char *key);

/*
 * data_cell_get is Bureaucratic, use data_cell_array_iterate instead 
 */
int data_cell_length(struct data_cell *c);
struct data_cell* data_cell_get(struct data_cell *c, int i);

#define data_cell_array_iterate(c, childcell)		\
	childcell = c->v.aval->items[0];				\
	for (int t_rsv_i = 0; t_rsv_i < c->v.aval->num; \
		 childcell = c->v.aval->items[++t_rsv_i])

#define iterate_data(c)											\
	for (int t_rsv_i = 0; t_rsv_i < c->v.aval->num; t_rsv_i++)

#endif	/* __MVTDATA_H__ */
