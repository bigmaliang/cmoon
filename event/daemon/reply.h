
#ifndef __MREPLY_H__
#define __MREPLY_H__

#include <stdint.h>		/* for uint32_t */

struct data_cell* reply_add_u32(struct queue_entry *q, const char *parent,
								const char *key, uint32_t val);
struct data_cell* reply_add_ulong(struct queue_entry *q, const char *parent,
								  const char *key, unsigned long val);
struct data_cell* reply_add_str(struct queue_entry *q, const char *parent,
								const char *key, const char *val);
struct data_cell* reply_add_array(struct queue_entry *q, const char *parent, const char *key);

struct data_cell* reply_trigger(struct queue_entry *q, uint32_t reply);

#endif	/* __MREPLY_H__ */
