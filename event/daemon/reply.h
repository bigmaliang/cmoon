
#ifndef __MREPLY_H__
#define __MREPLY_H__

#include <stdint.h>        /* for uint32_t */

struct data_cell* reply_trigger(struct queue_entry *q, uint32_t reply);

#endif    /* __MREPLY_H__ */
