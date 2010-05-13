#ifndef __MPACKET_H__
#define __MPACKET_H__

#include <stdint.h>		/* for uint32_t */

size_t unpack_data(const char *key, unsigned char *buf, size_t len,
                   struct data_cell **dataset);
size_t pack_data_u32(const char *key, uint32_t val, unsigned char *buf);
size_t pack_data_ulong(const char *key, unsigned long val, unsigned char *buf);
size_t pack_data_str(const char *key, const char *val, unsigned char *buf);
size_t pack_data_array(const char *key, struct data_cell *dataset,
                       unsigned char *buf, size_t maxsize);
size_t pack_data_any(const char *key, struct data_cell *dataset,
                     unsigned char *buf, size_t maxsize);

#endif	/* __MPACKET_H__ */
