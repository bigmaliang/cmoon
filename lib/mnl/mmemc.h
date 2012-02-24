#ifndef __MMEMC_H__
#define __MMEMC_H__

#include "mheads.h"

__BEGIN_DECLS

enum memc_op
{
    MMC_OP_SET = 0,
    MMC_OP_ADD,
    MMC_OP_REP,
    MMC_OP_APP,
    MMC_OP_PRE,
    MMC_OP_CAS,
    MMC_OP_INC,
    MMC_OP_DEC
};

memcached_return mmc_store(int op, const char *key, char *value,
                           size_t len, time_t exp, uint32_t flags);
memcached_return mmc_store_int(int op, const char *key, int value, time_t exp, uint32_t flags);
memcached_return mmc_count(int op, const char *key, uint32_t offset, uint64_t *value,
                           time_t exp, uint32_t flags);

char* mmc_get(const char *key, size_t *vallen, uint32_t *flags);
bool mmc_get_int(const char *key, int *value, uint32_t *flags);
memcached_return mmc_mget(const char **keys, char *vals[], int num,
                          size_t *val_len[], uint32_t *flags[]);
memcached_return mmc_delete(const char *key, time_t exp);



memcached_return mmc_storef(int op, char *value, size_t len, time_t exp,
                            uint32_t flags, const char *keyfmt, ...)
                            ATTRIBUTE_PRINTF(6, 7);
memcached_return mmc_storef_int(int op, int value, time_t exp,
                                uint32_t flags, const char *keyfmt, ...)
                                ATTRIBUTE_PRINTF(5, 6);
memcached_return mmc_countf(int op, uint32_t offset, uint64_t *value, time_t exp,
                            uint32_t flags, const char *keyfmt, ...)
                            ATTRIBUTE_PRINTF(6, 7);
char* mmc_getf(size_t *vallen, uint32_t *flags, const char *keyfmt, ...)
               ATTRIBUTE_PRINTF(3, 4);
bool mmc_getf_int(int *value, uint32_t *flags, const char *keyfmt, ...)
                  ATTRIBUTE_PRINTF(3, 4);
memcached_return mmc_deletef(time_t exp, const char *keyfmt, ...)
                 ATTRIBUTE_PRINTF(2, 3);

__END_DECLS
#endif    /* __MMEMC_H__ */
