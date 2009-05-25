#include "mheads.h"
#include "lheads.h"
#include "ocds.h"

static char cds_keys[CDS_DOMAIN_NUM][LEN_ST] =	{
	"n_user_space",
	"n_photo", "n_photo_album",
	"n_video", "n_video_d", "n_video_album",
	"n_blog"};
static char cds_tables[CDS_DOMAIN_NUM][LEN_ST] = {
	"home.home",
	"myphoto.photo_info", "myphoto.album_info",
	"myvideo.video", "myvideo.video", "myvideo.album",
	"myblog.blog"};
static char cds_cols_k[CDS_DOMAIN_NUM][LEN_ST] =	{
	"uid",
	"pid", "aid",
	"vid", "vid", "aid",
	"bid"};
static char cds_cols_v[CDS_DOMAIN_NUM][LEN_ST] =	{
	"view_num",
	"view_num", "view_num",
	"view_num", "up_num", "view_num",
	"view_num"};

static int cds_get_keyid(char *domain)
{
	int i;
	for (i = 0; i < CDS_DOMAIN_NUM; i++) {
		if (!strcmp(domain, cds_keys[i]))
			return i;
	}
	return -1;
}
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
	if (!strncmp(domain, "n_user", strlen("n_user"))) {
		nmdb_add_udp_server(db, NMDB_SERVER_USER, NMDB_PORT_USER);
	} else if (!strncmp(domain, "n_photo", strlen("n_photo"))) {
		nmdb_add_udp_server(db, NMDB_SERVER_PHOTO, NMDB_PORT_PHOTO);
	} else if (!strncmp(domain, "n_video", strlen("n_video"))) {
		nmdb_add_udp_server(db, NMDB_SERVER_VIDEO, NMDB_PORT_VIDEO);
	} else if (!strncmp(domain, "n_blog", strlen("n_blog"))) {
		nmdb_add_udp_server(db, NMDB_SERVER_BLOG, NMDB_PORT_BLOG);
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
	ret = cds_get_keyid(domain);
	if (ret < 0 || ret >= CDS_DOMAIN_NUM) {
		mtc_warn("domain %s invalid", domain);
		return RET_DBOP_INPUTE;
	}

	snprintf(fdb->sql, sizeof(fdb->sql), "SELECT %s FROM %s WHERE %s=%s;",
			 cds_cols_v[ret], cds_tables[ret], cds_cols_k[ret], key);
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
	ret = cds_get_keyid(domain);
	if (ret < 0 || ret >= CDS_DOMAIN_NUM) {
		mtc_warn("domain %s invalid", domain);
		return RET_DBOP_INPUTE;
	}
	
	snprintf(fdb->sql, sizeof(fdb->sql), "UPDATE %s SET %s=%s WHERE %s=%d;",
			 cds_tables[ret], cds_cols_v[ret], val, cds_cols_k[ret], id);
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
