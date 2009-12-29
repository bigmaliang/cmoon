#include <stdlib.h>		/* malloc() */
#include <stdint.h>		/* uint32_t and friends */
#include <stdbool.h>		/* bool */
#include <string.h>		/* memcpy() */
#include <arpa/inet.h>		/* htonl() and friends */

#include "net-const.h"
#include "data.h"
#include "packet.h"
#include "queue.h"

struct data_cell* reply_add_u32(struct queue_entry *q, const char *parent,
								const char *key, uint32_t val)
{
	if (q == NULL || key == NULL) return NULL;

	if (q->replydata == NULL)
		q->replydata = data_cell_alloc_array("root");

	return data_cell_add_u32(q->replydata, parent, key, val);
}

struct data_cell* reply_add_ulong(struct queue_entry *q, const char *parent,
								  const char *key, unsigned long val)
{
	if (q == NULL || key == NULL) return NULL;

	if (q->replydata == NULL)
		q->replydata = data_cell_alloc_array("root");

	return data_cell_add_ulong(q->replydata, parent, key, val);
}

struct data_cell* reply_add_str(struct queue_entry *q, const char *parent,
								const char *key, const char *val)
{
	if (q == NULL || key == NULL) return NULL;

	if (q->replydata == NULL)
		q->replydata = data_cell_alloc_array("root");

	return data_cell_add_str(q->replydata, parent, key, val);
}

struct data_cell* reply_add_array(struct queue_entry *q, const char *parent, const char *key)
{
	if (q == NULL || key == NULL) return NULL;

	if (q->replydata == NULL)
		q->replydata = data_cell_alloc_array("root");

	return data_cell_add_array(q->replydata, parent, key);
}

int reply_trigger(struct queue_entry *q, uint32_t reply)
{
	if (q == NULL) return 0;

	if (q->replydata == NULL) {
		q->req->reply_mini(q->req, reply);
		return 1;
	}
	
	unsigned char *buf = calloc(1, MAX_PACKET_LEN);
	if (buf == NULL) {
		q->req->reply_mini(q->req, REP_ERR_MEM);
		return 0;
	}

	size_t vsize;
	vsize = pack_data_array(NULL, q->replydata, buf, MAX_PACKET_LEN - RESERVE_SIZE);
	if (vsize == 0) goto error;
	
	* (uint32_t *) (buf+vsize) = htonl(DATA_TYPE_EOF);
	vsize += sizeof(uint32_t);
 
	q->req->reply_long(q->req, reply, buf, vsize);

	free(buf);
	return 1;
	
 error:
	q->req->reply_mini(q->req, REP_ERR_PACK);
	free(buf);
	return 0;
}
