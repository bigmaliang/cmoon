#include "mheads.h"

static memcached_st *l_mc = 0;
static bool l_inited = false;

static memcached_st* mmc_create()
{
    memcached_return rc;
    HDF *node;
    char *ip;
    int port;
    
    if (l_inited) return l_mc;
    l_inited = true;
    
    l_mc = memcached_create(NULL);
    if (l_mc == NULL) {
        mtc_err("create memcached struct failure!");
        return NULL;
    }

    node = hdf_get_obj(g_cfg, PRE_CFG_MEMC);
    if (node != NULL) {
        node = hdf_obj_child(node);
    } else {
        mtc_err("%s not found in config", PRE_CFG_MEMC);
        //memcached_free(l_mc);
        //l_mc = NULL;
        return NULL;
    }
    
    while (node != NULL) {
        ip = hdf_get_value(node, "ip", "127.0.0.1");
        port = hdf_get_int_value(node, "port", 0);
        rc = memcached_server_add(l_mc, ip, port);
        if (rc != MEMCACHED_SUCCESS) {
            mtc_err("init %s:%d %s", ip, port, memcached_strerror(l_mc, rc));
        }
        node = hdf_obj_next(node);
    }

    return l_mc;
}

memcached_return mmc_store(int op, const char *key, char *value, size_t len, time_t exp, uint32_t flags)
{
    memcached_st *mc;
    memcached_return rc;

    mc = mmc_create();
    if (mc == NULL) {
        mtc_err("memcached create failure!");
        return MEMCACHED_FAILURE;
    }
    size_t vallen = len;
    if (len == 0) {
        vallen = strlen(value);
    }
    switch (op) {
    case MMC_OP_SET:
        rc = memcached_set(mc, key, strlen(key), value, vallen, exp, flags);
        break;
    case MMC_OP_ADD:
        rc = memcached_add(mc, key, strlen(key), value, vallen, exp, flags);
        break;
    case MMC_OP_REP:
        rc = memcached_replace(mc, key, strlen(key), value, vallen, exp, flags);
        break;
    case MMC_OP_APP:
        rc = memcached_append(mc, key, strlen(key), value, vallen, exp, flags);
        break;
    case MMC_OP_PRE:
        rc = memcached_prepend(mc, key, strlen(key), value, vallen, exp, flags);
        break;
    case MMC_OP_CAS:
        rc = memcached_cas(mc, key, strlen(key), value, vallen, exp, flags, 0);
        break;
    default:
        rc = MEMCACHED_NOT_SUPPORTED;
        break;
    }
    if (rc == MEMCACHED_NOTSTORED && op == MMC_OP_ADD) {
        rc = MEMCACHED_SUCCESS;
    }
    if (rc != MEMCACHED_SUCCESS && (op == MMC_OP_APP || op == MMC_OP_PRE)) {
        mtc_warn("append or prepend '%s=%s' %s", key, value, memcached_strerror(mc, rc));
        rc = memcached_add(mc, key, strlen(key), value, vallen, exp, flags);
    }
    if (rc != MEMCACHED_SUCCESS) {
        mtc_err("store '%s=%s' %s", key, value, memcached_strerror(mc, rc));
    }
    //memcached_free(mc);
    return rc;
}
memcached_return mmc_store_int(int op, const char *key, int value, time_t exp, uint32_t flags)
{
    char cvalue[64];
    memset(cvalue, 0x0, sizeof(cvalue));
    sprintf(cvalue, "%d", value);
    return (mmc_store(op, key, cvalue, 0, exp, flags));
}

memcached_return mmc_count(int op, const char *key, uint32_t offset,
                           uint64_t *value, time_t exp, uint32_t flags)
{
    memcached_st *mc;
    memcached_return rc;
    char *dupkey = strdup(key);

    mc = mmc_create();
    if (mc == NULL) {
        mtc_err("memcached create failure!");
        return MEMCACHED_FAILURE;
    }
    uint64_t *plv, lvalue;
    plv = &lvalue;
    if (value != NULL)
        plv = value;
    switch (op) {
    case MMC_OP_INC:
        rc = memcached_increment(mc, key, strlen(key), offset, plv);
        break;
    case MMC_OP_DEC:
        rc = memcached_decrement(mc, key, strlen(key), offset, plv);
        break;
    default:
        rc = MEMCACHED_NOT_SUPPORTED;
        break;
    }
    if (rc == MEMCACHED_NOTFOUND) {
        char tval[LEN_ST];
        sprintf(tval, "%d", offset);
        rc = memcached_set(mc, dupkey, strlen(dupkey), tval, strlen(tval), exp, flags);
    }
    if (rc != MEMCACHED_SUCCESS) {
        mtc_err("count '%s' %s", key, memcached_strerror(mc, rc));
    }
    free(dupkey);
    //memcached_free(mc);
    return rc;
}

char* mmc_get(const char *key, size_t *vallen, uint32_t *flags)
{
    memcached_st *mc;
    memcached_return rc;
    char *value;

    mc = mmc_create();
    if (mc == NULL) {
        mtc_err("memcached create failure!");
        return NULL;
    }
    size_t lv, *plv;
    plv = &lv;
    if (vallen != NULL)
        plv = vallen;
    uint32_t lf, *plf;
    plf = &lf;
    if (flags != NULL)
        plf = flags;
    value = memcached_get(mc, key, strlen(key), plv, plf, &rc);
    if (value == NULL || *plv < 1) {
        mtc_info("get %s from %s", key, memcached_strerror(mc, rc));
        //memcached_free(mc);
        return NULL;
    }
    //memcached_free(mc);
    *(value+*plv) = '\0';
    return value;
}
bool mmc_get_int(const char *key, int *value, uint32_t *flags)
{
    char *cvalue = mmc_get(key, NULL, flags);
    if (cvalue == NULL) {
        *value = 0;
        return false;
    } else {
        *value = atoi(cvalue);
        free(cvalue);
        return true;
    }
}

memcached_return mmc_mget(const char **keys, char *vals[], int num,
                          size_t *val_len[], uint32_t *flags[])
{
    memcached_st *mc;
    memcached_return rc;
    size_t keys_len[num];

    mc = mmc_create();
    if (mc == NULL) {
        mtc_err("memcached create failure!");
        return MEMCACHED_FAILURE;
    }
    size_t *plen[num], len[num];
    uint32_t *pflg[num], flg[num];
    int i;
    if (val_len == NULL) {
        for (i = 0; i < num; i++) {
            plen[i] = &len[i];
        }
    }
    if (flags == NULL) {
        for (i = 0; i < num; i++) {
            pflg[i] = &flg[i];
        }
    }
    
    for (i = 0; i < num; i++) {
        keys_len[i] = strlen(keys[i]);
    }
    rc = memcached_mget(mc, keys, keys_len, (unsigned int)num);
    if (rc != MEMCACHED_SUCCESS) {
        mtc_info("%s", memcached_strerror(mc, rc));
        //memcached_free(mc);
        return rc;
    }
    for (i = 0; i < num; i++) {
        vals[i] = memcached_fetch(mc, (char*)keys[i], &keys_len[i], plen[i], pflg[i], &rc);
        if (rc == MEMCACHED_END) {
            mtc_info("%s", memcached_strerror(mc, rc));
            break;
        }
    }
    //memcached_free(mc);
    return rc;
}

memcached_return mmc_delete(const char *key, time_t exp)
{
    memcached_st *mc;
    memcached_return rc;

    mc = mmc_create();
    if (mc == NULL) {
        mtc_err("memcached create failure!");
        return MEMCACHED_FAILURE;
    }
    rc = memcached_delete(mc, key, strlen(key), exp);
    if (rc != MEMCACHED_SUCCESS) {
        mtc_info("%s %s", key, memcached_strerror(mc, rc));
    }
    //memcached_free(mc);
    return rc;
}


memcached_return mmc_storef(int op, char *value, size_t len, time_t exp, uint32_t flags,
                            const char *keyfmt, ...)
{
    char key[LEN_MMC_KEY];
    memcached_return rc;
    va_list ap;

    va_start(ap, keyfmt);
    vsnprintf(key, sizeof(key), keyfmt, ap);
    va_end(ap);

    rc = mmc_store(op, key, value, len, exp, flags);

    return rc;
}

memcached_return mmc_storef_int(int op, int value, time_t exp, uint32_t flags,
                                const char *keyfmt, ...)
{
    char key[LEN_MMC_KEY];
    memcached_return rc;
    va_list ap;

    va_start(ap, keyfmt);
    vsnprintf(key, sizeof(key), keyfmt, ap);
    va_end(ap);

    rc = mmc_store_int(op, key, value, exp, flags);

    return rc;
}

memcached_return mmc_countf(int op, uint32_t offset, uint64_t *value, time_t exp,
                            uint32_t flags,    const char *keyfmt, ...)
{
    char key[LEN_MMC_KEY];
    memcached_return rc;
    va_list ap;

    va_start(ap, keyfmt);
    vsnprintf(key, sizeof(key), keyfmt, ap);
    va_end(ap);

    rc = mmc_count(op, key, offset, value, exp, flags);

    return rc;
}

char* mmc_getf(size_t *vallen, uint32_t *flags, const char *keyfmt, ...)
{
    char key[LEN_MMC_KEY];
    char *res;
    va_list ap;

    va_start(ap, keyfmt);
    vsnprintf(key, sizeof(key), keyfmt, ap);
    va_end(ap);

    res = mmc_get(key, vallen, flags);

    return res;
}

bool mmc_getf_int(int *value, uint32_t *flags, const char *keyfmt, ...)
{
    char key[LEN_MMC_KEY];
    bool res;
    va_list ap;

    va_start(ap, keyfmt);
    vsnprintf(key, sizeof(key), keyfmt, ap);
    va_end(ap);

    res = mmc_get_int(key, value, flags);

    return res;
}

memcached_return mmc_deletef(time_t exp, const char *keyfmt, ...)
{
    char key[LEN_MMC_KEY];
    memcached_return rc;
    va_list ap;

    va_start(ap, keyfmt);
    vsnprintf(key, sizeof(key), keyfmt, ap);
    va_end(ap);

    rc = mmc_delete(key, exp);

    return rc;
}

