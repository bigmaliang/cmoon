#include <stdlib.h>        /* malloc() */
#include <stdint.h>        /* uint32_t and friends */
#include <string.h>        /* memcpy() */
#include <arpa/inet.h>        /* htonl() and friends */

#include "net-const.h"
#include "netutils.h"
#include "packet.h"

size_t unpack_data_str(unsigned char *buf, size_t len, char **val)
{
    const unsigned char *mykey;
    uint32_t ksize, vtype, vsize, ttsize;
    unsigned char *pos;
    
    if (!buf) return 0;

    pos = buf;
    
    vtype = * (uint32_t *) pos; vtype = ntohl(vtype);
    ttsize = sizeof(uint32_t);

    if (vtype != DATA_TYPE_STRING) return 0;

    pos = pos + sizeof(uint32_t);
    ksize = * (uint32_t *) pos; ksize = ntohl(ksize);

    pos = pos + sizeof(uint32_t);
    mykey = pos;

    pos = pos + ksize;
    ttsize += ksize + 2*sizeof(uint32_t);
    
    vsize = * (uint32_t *) pos; vsize = ntohl(vsize);
    pos = pos + sizeof(uint32_t);

    *val = (char*)pos;

    pos = pos + vsize;
    ttsize += sizeof(uint32_t) + vsize;

    return ttsize;
}

size_t unpack_hdf(unsigned char *buf, size_t len, HDF **hdf)
{
    size_t ttsize;
    char *val = NULL;
    
    if (!buf || !hdf) return 0;

    hdf_destroy(hdf);
    hdf_init(hdf);

    ttsize = unpack_data_str(buf, len, &val);
    if (val) hdf_read_string(*hdf, val);

    return ttsize;
}

size_t pack_data_str(const char *key, const char *val, unsigned char *buf, size_t len)
{
    size_t ksize = strlen(key);
    size_t vsize = 0;

    if (val) vsize = strlen(val)+1;

    if (vsize > len - RESERVE_SIZE) {
        vsize = len - RESERVE_SIZE;
    }
    
    * (uint32_t *) buf = htonl(DATA_TYPE_STRING);
    * ((uint32_t *) buf + 1) = htonl(ksize);
    memcpy(buf+8, key, ksize);
    * ((uint32_t *) (buf + 8 + ksize)) = htonl(vsize);
    if (vsize > 0) memcpy(buf+8+ksize+4, val, vsize);
    
    *(buf+8+ksize+4 + vsize) = '\0';

    return (8 + ksize + 4 + vsize);
}

size_t pack_hdf(HDF *hdf, unsigned char *buf, size_t len)
{
    size_t vsize;
    char *p;
    
    if (!hdf || !buf) return 0;
    
    NEOERR *err = hdf_write_string(hdf, &p);
    if (err != STATUS_OK) return 0;

    vsize = pack_data_str("root", p, buf, len);

    free(p);

    * (uint32_t *) (buf+vsize) = htonl(DATA_TYPE_EOF);
    vsize += sizeof(uint32_t);
    
    return vsize;
}
