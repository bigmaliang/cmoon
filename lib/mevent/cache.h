
#ifndef _CACHE_H
#define _CACHE_H

/* Generic cache layer. See cache.c for more information. */

#include <sys/types.h>     /* for size_t */
#include <stdint.h>        /* for int64_t */

#define MAX_CACHEKEY_LEN    1024

extern volatile time_t g_ctime;

struct cache {
    /* set directly by initialization */
    size_t numobjs;
    unsigned int flags;

    /* calculated */
    size_t hashlen;
    size_t chainlen;

    /* the cache data itself */
    struct cache_chain *table;
};

typedef struct cache Cache;

struct cache_chain {
    size_t len;
    struct cache_entry *first;
    struct cache_entry *last;
};

struct cache_entry {
    unsigned char *key;
    unsigned char *val;
    size_t ksize;
    size_t vsize;
    time_t expire;

    struct cache_entry *prev;
    struct cache_entry *next;
};


struct cache *cache_create(size_t numobjs, unsigned int flags);
int cache_free(struct cache *cd);
int cache_get(struct cache *cd, const unsigned char *key, size_t ksize,
              unsigned char **val, size_t *vsize);
int cache_set(struct cache *cd, const unsigned char *k, size_t ksize,
              const unsigned char *v, size_t vsize, int timeout);
int cache_del(struct cache *cd, const unsigned char *key, size_t ksize);
int cache_cas(struct cache *cd, const unsigned char *key, size_t ksize,
              const unsigned char *oldval, size_t ovsize,
              const unsigned char *newval, size_t nvsize);
int cache_incr(struct cache *cd, const unsigned char *key, size_t ksize,
               int64_t increment, int64_t *newval);

int cache_getf(struct cache *cd, unsigned char **val, size_t *vsize,
               const char *keyfmt, ...);
int cache_setf(struct cache *cd, const unsigned char *v, size_t vsize,
               int timeout, const char *keyfmt, ...);
int cache_delf(struct cache *cd, const char *keyfmt, ...);
int cache_incrf(struct cache *cd, int64_t increment, int64_t *newval,
                const char *keyfmt, ...);

#endif

