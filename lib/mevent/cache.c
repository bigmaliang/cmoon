
/* Generic cache layer.
 * It's a hash table with cache-style properties, keeping a (non-precise) size
 * and using a natural, per-chain LRU to do cleanups.
 * Cleanups are performed in place, when cache_set() gets called.
 */

#include <sys/types.h>        /* for size_t */
#include <stdint.h>        /* for [u]int*_t */
#include <stdlib.h>        /* for malloc() */
#include <string.h>        /* for memcpy()/memcmp() */
#include <stdio.h>        /* snprintf() */
#include <stdarg.h>        /* va_end() */
#include "cache.h"

volatile time_t g_ctime;

struct cache *cache_create(size_t numobjs, unsigned int flags)
{
    size_t i;
    struct cache *cd;
    struct cache_chain *c;

    cd = (struct cache *) malloc(sizeof(struct cache));
    if (cd == NULL)
        return NULL;

    cd->flags = flags;

    /* We calculate the hash size so we have 4 objects per bucket; 4 being
     * an arbitrary number. It's long enough to make LRU useful, and small
     * enough to make lookups fast. */
    cd->chainlen = 4;
    cd->numobjs = numobjs;
    cd->hashlen = numobjs / cd->chainlen;

    cd->table = (struct cache_chain *)
            malloc(sizeof(struct cache_chain) * cd->hashlen);
    if (cd->table == NULL) {
        free(cd);
        return NULL;
    }

    for (i = 0; i < cd->hashlen; i++) {
        c = cd->table + i;
        c->len = 0;
        c->first = NULL;
        c->last = NULL;
    }

    return cd;
}


int cache_free(struct cache *cd)
{
    size_t i;
    struct cache_chain *c;
    struct cache_entry *e, *n;

    for (i = 0; i < cd->hashlen; i++) {
        c = cd->table + i;
        if (c->first == NULL)
            continue;

        e = c->first;
        while (e != NULL) {
            n = e->next;
            free(e->key);
            free(e->val);
            free(e);
            e = n;
        }
    }

    free(cd->table);
    free(cd);
    return 1;
}


/*
 * The hash function used is the "One at a time" function, which seems simple,
 * fast and popular. Others for future consideration if speed becomes an issue
 * include:
 *  * FNV Hash (http://www.isthe.com/chongo/tech/comp/fnv/)
 *  * SuperFastHash (http://www.azillionmonkeys.com/qed/hash.html)
 *  * Judy dynamic arrays (http://judy.sf.net)
 *
 * A good comparison can be found at
 * http://eternallyconfuzzled.com/tuts/hashing.html#existing
 */

static uint32_t hash(const unsigned char *key, const size_t ksize)
{
    uint32_t h = 0;
    size_t i;

    for (i = 0; i < ksize; i++) {
        h += key[i];
        h += (h << 10);
        h ^= (h >> 6);
    }
    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);
    return h;
}


/* Looks up the given key in the chain. Returns NULL if not found, or a
 * pointer to the cache entry if it is. The chain can be empty. */
static struct cache_entry *find_in_chain(struct cache_chain *c,
        const unsigned char *key, size_t ksize)
{
    struct cache_entry *e;

    for (e = c->first; e != NULL; e = e->next) {
        if (ksize != e->ksize) {
            continue;
        }
        if (memcmp(key, e->key, ksize) == 0) {
            break;
        }
    }

    /* e will be either the found chain or NULL */
    return e;
}


/* Looks up the given key in the cache. Returns NULL if not found, or a
 * pointer to the cache entry if it is. Useful to avoid doing the calculation
 * in the open when the cache chain will not be needed. */
static struct cache_entry *find_in_cache(struct cache *cd,
        const unsigned char *key, size_t ksize)
{
    uint32_t h;
    struct cache_chain *c;

    h = hash(key, ksize) % cd->hashlen;
    c = cd->table + h;

    return find_in_chain(c, key, ksize);
}


/* Gets the matching value for the given key.  Returns 0 if no match was
 * found, or 1 otherwise. */
int cache_get(struct cache *cd, const unsigned char *key, size_t ksize,
        unsigned char **val, size_t *vsize)
{
    struct cache_entry *e;

    e = find_in_cache(cd, key, ksize);

    if (e == NULL) {
        *val = NULL;
        *vsize = 0;
        return 0;
    }

    if (e->expire > 0 && e->expire < g_ctime) {
        cache_del(cd, (unsigned char*)key, (size_t)ksize);
        *val = NULL;
        *vsize = 0;
        return 0;
    }

    *val = e->val;
    *vsize = e->vsize;

    return 1;
}


int cache_set(struct cache *cd, const unsigned char *key, size_t ksize,
        const unsigned char *val, size_t vsize, int timeout)
{
    int rv = 1;
    uint32_t h = 0;
    struct cache_chain *c;
    struct cache_entry *e, *new;
    unsigned char *v;

    h = hash(key, ksize) % cd->hashlen;
    c = cd->table + h;

    e = find_in_chain(c, key, ksize);

    if (e == NULL) {
        /* not found, create a new cache entry */
        new = malloc(sizeof(struct cache_entry));
        if (new == NULL) {
            rv = 0;
            goto exit;
        }

        new->ksize = ksize;
        new->vsize = vsize;
        if (timeout == 0) {
            new->expire = 0;
        } else {
            new->expire = g_ctime + timeout;
        }

        new->key = malloc(ksize);
        if (new->key == NULL) {
            free(new);
            rv = 0;
            goto exit;
        }
        memcpy(new->key, key, ksize);

        new->val = malloc(vsize);
        if (new->val == NULL) {
            free(new->key);
            free(new);
            rv = 0;
            goto exit;
        }
        memcpy(new->val, val, vsize);
        new->prev = NULL;
        new->next = NULL;

        /* and put it in */
        if (c->len == 0) {
            /* line is empty, just put it there */
            c->first = new;
            c->last = new;
            c->len = 1;
        } else if (c->len <= cd->chainlen) {
            /* slots are still available, put the entry first */
            new->next = c->first;
            c->first->prev = new;
            c->first = new;
            c->len += 1;
        } else {
            /* chain is full, we need to evict the last one */
            e = c->last;
            c->last = e->prev;
            c->last->next = NULL;
            free(e->key);
            free(e->val);
            free(e);

            new->next = c->first;
            c->first->prev = new;
            c->first = new;
        }
    } else {
        /* we've got a match, just replace the value in place */
        v = malloc(vsize);
        if (v == NULL) {
            rv = 0;
            goto exit;
        }
        free(e->val);
        e->val = v;
        e->vsize = vsize;
        memcpy(e->val, val, vsize);

        /* promote the entry to the top of the list if necessary */
        if (c->first != e) {
            if (c->last == e)
                c->last = e->prev;

            e->prev->next = e->next;
            if (e->next != NULL)
                e->next->prev = e->prev;
            e->prev = NULL;
            e->next = c->first;
            c->first->prev = e;
            c->first = e;
        }
    }

exit:
    return rv;
}


int cache_del(struct cache *cd, const unsigned char *key, size_t ksize)
{

    int rv = 1;
    uint32_t h = 0;
    struct cache_chain *c;
    struct cache_entry *e;

    h = hash(key, ksize) % cd->hashlen;
    c = cd->table + h;

    e = find_in_chain(c, key, ksize);

    if (e == NULL) {
        rv = 0;
        goto exit;
    }

    if (c->first == e) {
        c->first = e->next;
        if (e->next != NULL)
            e->next->prev = NULL;
    } else {
        e->prev->next = e->next;
        if (e->next != NULL)
            e->next->prev = e->prev;
    }

    if (c->last == e) {
        c->last = e->prev;
    }

    free(e->key);
    free(e->val);
    free(e);

    c->len -= 1;

exit:
    return rv;
}


/* Performs a cache compare-and-swap.
 * Returns -2 if there was an error, -1 if the key is not in the cache, 0 if
 * the old value does not match, and 1 if the CAS was successful. */
int cache_cas(struct cache *cd, const unsigned char *key, size_t ksize,
        const unsigned char *oldval, size_t ovsize,
        const unsigned char *newval, size_t nvsize)
{
    int rv = 1;
    struct cache_entry *e;
    unsigned char *buf;

    e = find_in_cache(cd, key, ksize);

    if (e == NULL) {
        rv = -1;
        goto exit;
    }

    if (e->vsize != ovsize) {
        rv = 0;
        goto exit;
    }

    if (memcmp(e->val, oldval, ovsize) != 0) {
        rv = 0;
        goto exit;
    }

    buf = malloc(nvsize);
    if (buf == NULL) {
        rv = -2;
        goto exit;
    }

    memcpy(buf, newval, nvsize);
    free(e->val);
    e->val = buf;
    e->vsize = nvsize;

exit:
    return rv;
}


/* Increment the value associated with the given key by the given increment.
 * The increment is a signed 64 bit value, and the value size must be >= 8
 * bytes.
 * Returns:
 *    1 if the increment succeeded.
 *   -1 if the value was not in the cache.
 *   -2 if the value was not null terminated.
 *   -3 if there was a memory error.
 *
 * The new value will be set in the newval parameter if the increment was
 * successful.
 */
int cache_incr(struct cache *cd, const unsigned char *key, size_t ksize,
        int64_t increment, int64_t *newval)
{
    unsigned char *val;
    int64_t intval;
    size_t vsize;
    struct cache_entry *e;

    e = find_in_cache(cd, key, ksize);

    if (e == NULL)
        return -1;

    val = e->val;
    vsize = e->vsize;

    /* the value must be a NULL terminated string, otherwise strtoll might
     * cause a segmentation fault */
    if (val && val[vsize - 1] != '\0')
        return -2;

    intval = strtoll((char *) val, NULL, 10);
    intval = intval + increment;

    /* The max value for an unsigned long long is 18446744073709551615,
     * and strlen('18446744073709551615') = 20, so if the value is smaller
     * than 24 (just in case) we create a new buffer. */
    if (vsize < 24) {
        unsigned char *nv = malloc(24);
        if (nv == NULL)
            return -3;
        free(val);
        e->val = val = nv;
        e->vsize = vsize = 24;
    }

    snprintf((char *) val, vsize, "%23lld", (long long int) intval);
    *newval = intval;

    return 1;
}

int cache_getf(struct cache *cd, unsigned char **val, size_t *vsize,
               const char *keyfmt, ...)
{
    char key[MAX_CACHEKEY_LEN];
    va_list ap;
    int r;

    va_start(ap, keyfmt);
    r = vsnprintf(key, MAX_CACHEKEY_LEN, keyfmt, ap);
    va_end(ap);

    return cache_get(cd, (unsigned char*)key, (size_t)r, val, vsize);
}

int cache_setf(struct cache *cd, const unsigned char *v, size_t vsize,
               int timeout, const char *keyfmt, ...)
{
    char key[MAX_CACHEKEY_LEN];
    va_list ap;
    int r;

    va_start(ap, keyfmt);
    r = vsnprintf(key, MAX_CACHEKEY_LEN, keyfmt, ap);
    va_end(ap);

    return cache_set(cd, (unsigned char*)key, (size_t)r, v, vsize, timeout);
}

int cache_delf(struct cache *cd, const char *keyfmt, ...)
{
    char key[MAX_CACHEKEY_LEN];
    va_list ap;
    int r;

    va_start(ap, keyfmt);
    r = vsnprintf(key, MAX_CACHEKEY_LEN, keyfmt, ap);
    va_end(ap);

    return cache_del(cd, (unsigned char*)key, (size_t)r);
}

int cache_incrf(struct cache *cd, int64_t increment, int64_t *newval,
                const char *keyfmt, ...)
{
    char key[MAX_CACHEKEY_LEN];
    va_list ap;
    int r;

    va_start(ap, keyfmt);
    r = vsnprintf(key, MAX_CACHEKEY_LEN, keyfmt, ap);
    va_end(ap);

    return cache_incr(cd, (unsigned char*)key, (size_t)r, increment, newval);
}
