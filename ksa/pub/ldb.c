#include "mheads.h"
#include "lheads.h"

void ldb_opfinish_json(int ret, HDF *hdf, mdb_conn *conn)
{
	if (ret < RET_RBTOP_MAX_M) {
		mdb_opfinish_json(ret, hdf, conn);
		return;
	}

	switch (ret) {
	default:
		hdf_set_value(hdf, PRE_OUTPUT".errmsg", "数据库操作错误");
		break;
	}
	if (conn != NULL) {
		mdb_destroy(conn);
	}
	mjson_output_hdf(hdf, 0);
	exit(ret);
}
