#include "mheads.h"
#include "lheads.h"
#include "oids.h"

int ids_dbt_init(ids_db_t **dbt)
{
	HDF *node = hdf_get_obj(g_cfg, CFG_IDS_DB);
	if (node == NULL) {
		mtc_err("no %s configed", CFG_IDS_DB);
		return RET_DBOP_INPUTE;
	}
	
	ids_db_t *ldb = calloc(1, sizeof(ids_db_t));
	if (ldb == NULL) return RET_DBOP_MEMALLOCE;

	ldb->dbs = (fdb_t**)calloc(IDS_MAX_DBS, sizeof(fdb_t*));
	if (ldb->dbs == NULL) {
		free(ldb);
		return RET_DBOP_MEMALLOCE;
	}

	fdb_t *sdb;
	node = hdf_obj_child(node);
	while (node != NULL) {
		sdb = NULL;
		if (fdb_init_long(&sdb, hdf_get_value(node, "ip", "127.0.0.1"),
						  hdf_get_value(node, "user", "test"),
						  hdf_get_value(node, "pass", "test"),
						  hdf_get_value(node, "name", "test"),
						  (unsigned int)hdf_get_int_value(node, "port", 0)) != RET_DBOP_OK) {
			mtc_err("connect %d error", hdf_obj_value(node));
		} else {
			ldb->dbs[ldb->num] = sdb;
			ldb->num++;
		}
		node = hdf_obj_next(node);
		if (ldb->num >= IDS_MAX_DBS) break;
	}

	*dbt = ldb;

	return RET_DBOP_OK;
}

void dbt_free(ids_db_t *dbt)
{
	if (dbt == NULL) return;
	for (int i = 0; i < dbt->num; i++) {
		fdb_free(&dbt->dbs[i]);
	}
	free(dbt);
}

int ids_get_data(HDF *hdf, ids_db_t *dbt)
{
	int ret;

	int i = neo_rand(dbt->num);
	fdb_t *fdb = dbt->dbs[i];
	
	if (fdb == NULL || fdb->conn == NULL) {
		mtc_err("connect error");
		return RET_DBOP_CONNECTE;
	}

	char *op = hdf_get_value(hdf, PRE_QUERY".op", NULL);
	char *key = hdf_get_value(hdf, PRE_QUERY".key", NULL);
	if (op == NULL || !mutil_isdigit(key)) {
		mtc_err("input error %s %s", op, key);
		return RET_DBOP_INPUTE;
	}

	char cfgkey[256];
	snprintf(cfgkey, sizeof(cfgkey), "%s.%s", CFG_IDS_TABLE, op);
	HDF *node = hdf_get_obj(g_cfg, cfgkey);
	if (node == NULL) {
		mtc_warn("op %s invalid", op);
		return RET_DBOP_INPUTE;
	}
	char *table, *keyc, *valc;
	table = hdf_get_value(node, "table", "null.blog");
	keyc = hdf_get_value(node, "keycol", "bid");
	valc = hdf_get_value(node, "valcol", "view_num");
	
	snprintf(fdb->sql, sizeof(fdb->sql), "SELECT %s FROM %s WHERE %s=%s;",
			 valc, table, keyc, key);
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
