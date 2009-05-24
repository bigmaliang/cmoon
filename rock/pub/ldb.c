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
		strcpy(res, "用户未登录");
		break;
	case RET_RBTOP_LIMITE:
		strcpy(res, "用户无权限");
		break;
	case RET_RBTOP_EXISTE:
		strcpy(res, "资源已存在");
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
	exit(ret);
}

void ldb_opfinish_json(int ret, HDF *hdf, mdb_conn *conn)
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
	mjson_output_hdf(hdf);
	
	if (conn != NULL) {
		mdb_destroy(conn);
	}
	exit(ret);
}
