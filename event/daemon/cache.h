
#ifndef _CACHE_H
#define _CACHE_H

/* Generic cache layer. See cache.c for more information. */

#include <sys/types.h>		/* for size_t */
#include <stdint.h>		/* for int64_t */


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

	struct cache_entry *prev;
	struct cache_entry *next;
};


struct cache *cache_create(size_t numobjs, unsigned int flags);
int cache_free(struct cache *cd);
int cache_get(struct cache *cd, const unsigned char *key, size_t ksize,
		unsigned char **val, size_t *vsize);
int cache_set(struct cache *cd, const unsigned char *k, size_t ksize,
		const unsigned char *v, size_t vsize);
int cache_del(struct cache *cd, const unsigned char *key, size_t ksize);
int cache_cas(struct cache *cd, const unsigned char *key, size_t ksize,
		const unsigned char *oldval, size_t ovsize,
		const unsigned char *newval, size_t nvsize);
int cache_incr(struct cache *cd, const unsigned char *key, size_t ksize,
		int64_t increment, int64_t *newval);

#endif

