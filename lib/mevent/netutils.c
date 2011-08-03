
#include <arpa/inet.h>        /* htonl() and friends */
#include "netutils.h"


/* ntohll() and htonll() are not standard, so we define it using an UGLY trick
 * because there is no standard way to check for endianness at runtime! */
uint64_t ntohll(uint64_t x)
{
    static int endianness = 0;

    /* determine the endianness by checking how htonl() behaves; use -1
     * for little endian and 1 for big endian */
    if (endianness == 0) {
        if (htonl(1) == 1)
            endianness = 1;
        else
            endianness = -1;
    }

    if (endianness == 1) {
        /* big endian */
        return x;
    }

    /* little endian */
    return ( ntohl( (x >> 32) & 0xFFFFFFFF ) | \
            ( (uint64_t) ntohl(x & 0xFFFFFFFF) ) << 32 );
}

uint64_t htonll(uint64_t x)
{
    static int endianness = 0;

    /* determine the endianness by checking how htonl() behaves; use -1
     * for little endian and 1 for big endian */
    if (endianness == 0) {
        if (htonl(1) == 1)
            endianness = 1;
        else
            endianness = -1;
    }

    if (endianness == 1) {
        /* big endian */
        return x;
    }

    /* little endian */
    return ( htonl( (x >> 32) & 0xFFFFFFFF ) | \
            ( (uint64_t) htonl(x & 0xFFFFFFFF) ) << 32 );
}

