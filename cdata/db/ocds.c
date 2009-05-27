#include "mheads.h"
#include "lheads.h"
#include "ocds.h"

static int cds_nmkey2sqlkey(char *key, char *val, int len, int *id)
{
	int slen;
	char *p = strrchr(key, '_');
	if (p == NULL)
		return RET_DBOP_INPUTE;
	slen = p-key;
	if (slen > len-1) slen = len-1;
	
	*id = atoi(p+1);
	memcpy(val, key, slen);
	*(val+slen+1) = '\0';
	
	return RET_DBOP_OK;
}

int cds_parse_key(char *key, ULIST **list)
{
	if (key == NULL) {
		return RET_DBOP_INPUTE;
	}

	if (!reg_search("^([0-9]+,?)+$", key)) {
		mtc_err("key %s illegal", key);
		return RET_DBOP_ERROR;
	}

	NEOERR *err = string_array_split(list, key, ",", 100);
	if (err != STATUS_OK) {
		mtc_err("split string error");
		return RET_DBOP_ERROR;
	}

	return RET_DBOP_OK;
}

int cds_parse_domain(char *domain, ULIST **list)
{
	if (domain == NULL) {
		return RET_DBOP_INPUTE;
	}

	NEOERR *err = string_array_split(list, domain, ",", 100);
	if (err != STATUS_OK) {
		mtc_err("split string error");
		return RET_DBOP_ERROR;
	}

	return RET_DBOP_OK;
}

int cds_add_udp_server(nmdb_t *db, char *domain)
{
	if(domain == NULL) {
		mtc_warn("domain null");
		return RET_DBOP_INPUTE;
	}

	char key[64];
	strcpy(key, CFG_NMDB);
	strcat(key, ".");
	/* strlen("n_user") = 6 */
	strncat(key, domain, 6);
	HDF *node = hdf_get_obj(g_cfg, key);
	if (node != NULL) {
		char *ip = hdf_get_value(node, "ip", "127.0.0.1");
		int port = hdf_get_int_value(node, "port", 26010);
		nmdb_add_udp_server(db, ip, port);
	} else {
		mtc_warn("unknown domain %s", domain);
		return RET_DBOP_ERROR;
	}
	
	return RET_DBOP_OK;
}

int cds_get_data(HDF *hdf, char *key, char *domain, char *hdfkey, fdb_t *fdb)
{
	int ret;
	
	if (fdb->conn == NULL) {
		mtc_err("connect error");
		return RET_DBOP_CONNECTE;
	}

	if (key == NULL || domain == NULL || hdfkey == NULL)
		return RET_DBOP_INPUTE;

	char cfgkey[256];
	snprintf(cfgkey, sizeof(cfgkey), "%s.%s", CFG_CDS_TABLE, domain);
	HDF *node = hdf_get_obj(g_cfg, cfgkey);
	if (node == NULL) {
		mtc_warn("domain %s invalid", domain);
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
		return RET_DBOP_ERROR;
	}
	if (fdb_fetch_row(fdb) != RET_DBOP_OK) {
		mtc_err("fetch %s error: %s", fdb->sql, fdb_error(fdb));
		return RET_DBOP_ERROR;
	}
	mtc_dbg("set value %s ...", fdb->row[0]);
	hdf_set_valuef(hdf, "%s.value=%s", hdfkey, fdb->row[0]);
	char thdfkey[LEN_ST];
	snprintf(thdfkey, sizeof(thdfkey), "%s.value", hdfkey);
	hdf_set_attr(hdf, thdfkey, "type", "int");

	return RET_DBOP_OK;
}

int cds_store_increment(fdb_t *fdb, char *key, char *val)
{
	char domain[LEN_NMDB_KEY];
	int id;
	int ret;
	
	if (fdb->conn == NULL) {
		mtc_err("connect error");
		return RET_DBOP_CONNECTE;
	}
	if (!mutil_isdigit(val)) {
		mtc_warn("%s not digit", val);
		return RET_DBOP_INPUTE;
	}
	ret = cds_nmkey2sqlkey(key, domain, sizeof(domain), &id);
	if (ret != RET_DBOP_OK) {
		mtc_warn("%s not hifly instant data key", key);
		return RET_DBOP_INPUTE;
	}
	
	char cfgkey[256];
	snprintf(cfgkey, sizeof(cfgkey), "%s.%s", CFG_CDS_TABLE, domain);
	HDF *node = hdf_get_obj(g_cfg, cfgkey);
	if (node == NULL) {
		mtc_warn("domain %s invalid", domain);
		return RET_DBOP_INPUTE;
	}
	char *table, *keyc, *valc;
	table = hdf_get_value(node, "table", "null.blog");
	keyc = hdf_get_value(node, "keycol", "bid");
	valc = hdf_get_value(node, "valcol", "view_num");

	snprintf(fdb->sql, sizeof(fdb->sql), "UPDATE %s SET %s=%s WHERE %s=%d;",
			 table, valc, val, keyc, id);
	ret = fdb_exec(fdb);
	if (ret != RET_DBOP_OK) {
		mtc_err("exec %s error: %s", fdb->sql, fdb_error(fdb));
		return RET_DBOP_ERROR;
	}
	if (fdb_affect_rows(fdb) <= 0) {
		mtc_err("exec %s, key not exist", fdb->sql);
		return RET_DBOP_ERROR;
	}

	return RET_DBOP_OK;
}
