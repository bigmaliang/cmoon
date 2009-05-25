#include "mheads.h"
#include "lheads.h"
#include "oids.h"

#define IDS_SLAVE_DB_NUM	3
char g_dbip_slave[IDS_SLAVE_DB_NUM][LEN_ST] = {"192.168.8.84", "192.168.8.84", "192.168.8.84"};

static char ids_keys[IDS_DOMAIN_NUM][LEN_ST] = {"n_user_message"};
static char ids_tables[IDS_DOMAIN_NUM][LEN_ST] = {"home.home"};
static char ids_cols_k[IDS_DOMAIN_NUM][LEN_ST] = {"uid"};
static char ids_cols_v[IDS_DOMAIN_NUM][LEN_ST] = {"view_num"};

static int ids_get_keyid(char *domain)
{
	int i;
	for (i = 0; i < IDS_DOMAIN_NUM; i++) {
		if (!strcmp(domain, ids_keys[i]))
			return i;
	}
	return -1;
}

int ids_fdb_init(fdb_t **fdb)
{
	int i = neo_rand(IDS_SLAVE_DB_NUM);
	return ldb_init(fdb, g_dbip_slave[i], NULL);
}

int ids_get_data(HDF *hdf, fdb_t *fdb)
{
	int ret;
	
	if (fdb->conn == NULL) {
		mtc_err("connect error");
		return RET_DBOP_CONNECTE;
	}

	char *op = hdf_get_value(hdf, PRE_QUERY".op", NULL);
	char *key = hdf_get_value(hdf, PRE_QUERY".key", NULL);
	if (op == NULL || !mutil_isdigit(key)) {
		mtc_err("input error %s %s", op, key);
		return RET_DBOP_INPUTE;
	}

	ret = ids_get_keyid(op);
	if (ret < 0 || ret >= IDS_DOMAIN_NUM) {
		mtc_warn("op %s invalid", op);
		return RET_DBOP_INPUTE;
	}

	snprintf(fdb->sql, sizeof(fdb->sql), "SELECT %s FROM %s WHERE %s=%s;",
			 ids_cols_v[ret], ids_tables[ret], ids_cols_k[ret], key);
	ret = fdb_exec(fdb);
	if (ret != RET_DBOP_OK) {
		mtc_err("exec %s error: %s", fdb->sql, fdb_error(fdb));
		return RET_DBOP_SELECTE;
	}
	if (fdb_fetch_row(fdb) != RET_DBOP_OK) {
		mtc_err("fetch %s error: %s", fdb->sql, fdb_error(fdb));
		return RET_DBOP_EXIST;
	}
	
	hdf_set_value(hdf, PRE_OUTPUT".value", fdb->row[0]);
	hdf_set_attr(hdf, PRE_OUTPUT".value", "type", "int");

	return RET_DBOP_OK;
}
