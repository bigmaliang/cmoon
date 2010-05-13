#ifndef __MPACKET_H__
#define __MPACKET_H__

#include <stdint.h>		/* for uint32_t */

#include "ClearSilver.h"

size_t unpack_data_str(unsigned char *buf, size_t len, char **val);
size_t unpack_hdf(unsigned char *buf, size_t len, HDF **hdf);

size_t pack_data_str(const char *key, const char *val, unsigned char *buf);
size_t pack_hdf(HDF *hdf, unsigned char *buf);

#endif	/* __MPACKET_H__ */
