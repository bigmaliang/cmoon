#ifndef __MPACKET_H__
#define __MPACKET_H__

#include <stdint.h>        /* for uint32_t */

#include "ClearSilver.h"

/*
 * unpacket a string into hdf dataset
 * buf: input hdf string
 */
size_t unpack_hdf(unsigned char *buf, size_t len, HDF **hdf);
size_t unpack_data_str(unsigned char *buf, size_t len, char **val);

/*
 * packet a hdf's CHILD to transable string
 * hdf: input hdf dataset, should contain child,
 *      it's own value will be ignore
 */
size_t pack_hdf(HDF *hdf, unsigned char *buf, size_t len);
size_t pack_data_str(const char *key, const char *val,
                     unsigned char *buf, size_t len);

#endif    /* __MPACKET_H__ */
