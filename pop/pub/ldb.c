#include "mheads.h"
#include "lheads.h"

static void get_errmsg(int ret, char *res)
{
	switch (ret) {
	case RET_RBTOP_LOGINUSR:
		strcpy(res, "用户不存在");
		break;
	case RET_RBTOP_LOGINPSW:
		strcpy(res, "密码不正确");
		break;
	case RET_RBTOP_RELEASED:
		strcpy(res, "号码使用中");
		break;
	case RET_RBTOP_NOTLOGIN:
		strcpy(res, "敏感操作, 请先登录");
		break;
	case RET_RBTOP_LIMITE:
		strcpy(res, "用户无权限");
		break;
	case RET_RBTOP_NEXIST:
		strcpy(res, "资源不存在");
		break;
	case RET_RBTOP_EXISTE:
		strcpy(res, "资源已存在");
		break;
	case RET_RBTOP_IMGPROE:
		strcpy(res, "处理图片失败");
		break;
	default:
		strcpy(res, "数据库操作错误");
		break;
	}
}


void ldb_opfinish(int ret, HDF *hdf, mdb_conn *conn,
				  char *target, char *url, bool header)
{
	char msg[LEN_SM];
	
	if (ret == RET_RBTOP_OK) {
		return;
	}
	
	if (ret < RET_RBTOP_MAX_M) {
		mdb_opfinish(ret, hdf, conn, target, url, header);
		return;
	}

	get_errmsg(ret, msg);
	mutil_redirect(msg, target, url, header);
	
	if (conn != NULL) {
		mdb_destroy(conn);
	}
	//exit(ret);
}

void ldb_opfinish_json(int ret, HDF *hdf, mdb_conn *conn, time_t second)
{
	char msg[LEN_SM];
	
	if (ret == RET_RBTOP_OK) {
		hdf_set_value(hdf, PRE_SUCCESS, "1");
		return;
	}
	
	if (ret < RET_RBTOP_MAX_M) {
		mdb_opfinish_json(ret, hdf, conn);
		return;
	}

	hdf_remove_tree(hdf, PRE_SUCCESS);
	get_errmsg(ret, msg);
	hdf_set_value(hdf, PRE_ERRMSG, msg);
	
	if (conn != NULL) {
		mdb_destroy(conn);
	}
	//exit(ret);
}


int ldb_init(HASH **dbh)
{
	HDF *node;
	HASH *ldbh;
	NEOERR *err;
	int ret;

	node = hdf_get_obj(g_cfg, "Db.Dsn");
	if (node == NULL) {
		mtc_err("Db config not found");
		return RET_RBTOP_INITE;
	}
	
	err = hash_init(&ldbh, hash_str_hash, hash_str_comp);
	RETURN_V_NOK(err, RET_RBTOP_INITE);

	node = hdf_obj_child(node);
	mdb_conn *conn;
	bool filled = false;
	while (node != NULL) {
		ret = mdb_init(&conn, hdf_obj_value(node));
		if (ret == RET_RBTOP_OK) {
			hash_insert(ldbh, (void*)strdup(hdf_obj_name(node)), (void*)conn);
			filled = true;
		}
		
		node = hdf_obj_next(node);
	}

	if (!filled) {
		mtc_err("no valid db connection");
		return RET_RBTOP_INITE;
	}
	*dbh = ldbh;
	return RET_RBTOP_OK;
}

void ldb_destroy(HASH *dbh)
{
	char *key = NULL;
	
	mdb_conn *conn = (mdb_conn*)hash_next(dbh, (void**)&key);

	while (conn != NULL) {
		mdb_destroy(conn);
		conn = hash_next(dbh, (void**)&key);
	}

	hash_destroy(&dbh);
}

