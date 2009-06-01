#include "mheads.h"

memcached_return mmc_store(int op, const char *key, char *value, size_t len, time_t exp, uint32_t flags)
{
	memcached_st *mc;
	memcached_return rc;

	mc = memcached_create(NULL);
	if (mc == NULL) {
		mtc_err("create memcached struct failure!");
		return MEMCACHED_FAILURE;
	}
	rc = memcached_server_add(mc, MEMC_IP, atoi(MEMC_PORT));
	if (rc != MEMCACHED_SUCCESS) {
		mtc_err("init %s:%s %s", MEMC_IP, MEMC_PORT, memcached_strerror(mc, rc));
		memcached_free(mc);
		return rc;
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
		mtc_warn("%s:%s append or prepend '%s=%s' %s", MEMC_IP, MEMC_PORT, key, value, memcached_strerror(mc, rc));
		rc = memcached_add(mc, key, strlen(key), value, vallen, exp, flags);
	}
	if (rc != MEMCACHED_SUCCESS) {
		mtc_err("%s:%s store '%s=%s' %s", MEMC_IP, MEMC_PORT, key, value, memcached_strerror(mc, rc));
	}
	memcached_free(mc);
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

	mc = memcached_create(NULL);
	if (mc == NULL) {
		mtc_err("create memcached struct failure!");
		return MEMCACHED_FAILURE;
	}
	rc = memcached_server_add(mc, MEMC_IP, atoi(MEMC_PORT));
	if (rc != MEMCACHED_SUCCESS) {
		mtc_err("init %s:%s %s", MEMC_IP, MEMC_PORT, memcached_strerror(mc, rc));
		memcached_free(mc);
		return rc;
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
		mtc_err("%s:%s count '%s' %s", MEMC_IP, MEMC_PORT, key, memcached_strerror(mc, rc));
	}
	free(dupkey);
	memcached_free(mc);
	return rc;
}

char* mmc_get(const char *key, size_t *vallen, uint32_t *flags)
{
	memcached_st *mc;
	memcached_return rc;
	char *value;

	mc = memcached_create(NULL);
	if (mc == NULL) {
		mtc_err("create memcached struct failure!");
		return NULL;
	}
	rc = memcached_server_add(mc, MEMC_IP, atoi(MEMC_PORT));
	if (rc != MEMCACHED_SUCCESS) {
		mtc_err("init %s:%s %s", MEMC_IP, MEMC_PORT, memcached_strerror(mc, rc));
		memcached_free(mc);
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
		mtc_info("get %s from %s:%s %s", key, MEMC_IP, MEMC_PORT, memcached_strerror(mc, rc));
		memcached_free(mc);
		return NULL;
	}
	memcached_free(mc);
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

memcached_return mmc_mget(char **keys, char *vals[], int num,
						  size_t *val_len[], uint32_t *flags[])
{
	memcached_st *mc;
	memcached_return rc;
	size_t keys_len[num];

	mc = memcached_create(NULL);
	if (mc == NULL) {
		mtc_err("create memcached struct failure!");
		return MEMCACHED_FAILURE;
	}
	rc = memcached_server_add(mc, MEMC_IP, atoi(MEMC_PORT));
	if (rc != MEMCACHED_SUCCESS) {
		mtc_err("%s:%s %s", MEMC_IP, MEMC_PORT, memcached_strerror(mc, rc));
		memcached_free(mc);
		return rc;
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
		mtc_info("%s:%s %s", MEMC_IP, MEMC_PORT, memcached_strerror(mc, rc));
		memcached_free(mc);
		return rc;
	}
	for (i = 0; i < num; i++) {
		vals[i] = memcached_fetch(mc, keys[i], &keys_len[i], plen[i], pflg[i], &rc);
		if (rc == MEMCACHED_END) {
			mtc_info("%s:%s %s", MEMC_IP, MEMC_PORT, memcached_strerror(mc, rc));
			break;
		}
	}
	memcached_free(mc);
	return rc;
}

memcached_return mmc_delete(const char *key, time_t exp)
{
	memcached_st *mc;
	memcached_return rc;

	mc = memcached_create(NULL);
	if (mc == NULL) {
		mtc_err("create memcached struct failure!");
		return MEMCACHED_FAILURE;
	}
	rc = memcached_server_add(mc, MEMC_IP, atoi(MEMC_PORT));
	if (rc != MEMCACHED_SUCCESS) {
		mtc_err("init %s:%s %s", MEMC_IP, MEMC_PORT, memcached_strerror(mc, rc));
		memcached_free(mc);
		return rc;
	}
	rc = memcached_delete(mc, key, strlen(key), exp);
	if (rc != MEMCACHED_SUCCESS) {
		mtc_info("%s:%s %s %s", MEMC_IP, MEMC_PORT, key, memcached_strerror(mc, rc));
	}
	memcached_free(mc);
	return rc;
}


memcached_return mmc_storef(int op, char *value, size_t len, time_t exp, uint32_t flags,
							const char *keyfmt, ...)
{
	char *key;
	memcached_return rc;
	va_list ap;

	va_start(ap, keyfmt);
	key = vsprintf_alloc(keyfmt, ap);
	va_end(ap);
	if (key == NULL) return MEMCACHED_MEMORY_ALLOCATION_FAILURE;

	rc = mmc_store(op, key, value, len, exp, flags);
	free(key);

	return rc;
}

memcached_return mmc_storef_int(int op, int value, time_t exp, uint32_t flags,
								const char *keyfmt, ...)
{
	char *key;
	memcached_return rc;
	va_list ap;

	va_start(ap, keyfmt);
	key = vsprintf_alloc(keyfmt, ap);
	va_end(ap);
	if (key == NULL) return MEMCACHED_MEMORY_ALLOCATION_FAILURE;

	rc = mmc_store_int(op, key, value, exp, flags);
	free(key);

	return rc;
}

memcached_return mmc_countf(int op, uint32_t offset, uint64_t *value, time_t exp,
							uint32_t flags,	const char *keyfmt, ...)
{
	char *key;
	memcached_return rc;
	va_list ap;

	va_start(ap, keyfmt);
	key = vsprintf_alloc(keyfmt, ap);
	va_end(ap);
	if (key == NULL) return MEMCACHED_MEMORY_ALLOCATION_FAILURE;

	rc = mmc_count(op, key, offset, value, exp, flags);
	free(key);

	return rc;
}

char* mmc_getf(size_t *vallen, uint32_t *flags, const char *keyfmt, ...)
{
	char *key;
	char *res;
	va_list ap;

	va_start(ap, keyfmt);
	key = vsprintf_alloc(keyfmt, ap);
	va_end(ap);
	if (key == NULL) return NULL;

	res = mmc_get(key, vallen, flags);
	free(key);

	return res;
}

bool mmc_getf_int(int *value, uint32_t *flags, const char *keyfmt, ...)
{
	char *key;
	bool res;
	va_list ap;

	va_start(ap, keyfmt);
	key = vsprintf_alloc(keyfmt, ap);
	va_end(ap);
	if (key == NULL) return false;

	res = mmc_get_int(key, value, flags);
	free(key);

	return res;
}

memcached_return mmc_deletef(time_t exp, const char *keyfmt, ...)
{
	char *key;
	memcached_return rc;
	va_list ap;

	va_start(ap, keyfmt);
	key = vsprintf_alloc(keyfmt, ap);
	va_end(ap);
	if (key == NULL) return MEMCACHED_MEMORY_ALLOCATION_FAILURE;

	rc = mmc_delete(key, exp);
	free(key);

	return rc;
}

