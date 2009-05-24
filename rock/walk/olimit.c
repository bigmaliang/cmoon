#include "mheads.h"
#include "lheads.h"
#include "ofile.h"
#include "member.h"

int limit_get_limits(HDF *hdf, mdb_conn *conn)
{
	PRE_DBOP(hdf, conn);

	int ret;
	char cols[LEN_SM];
	int count, offset;

	mmisc_set_count(hdf, conn, "fileinfo", "1=1");
	mmisc_get_offset(hdf, &count, &offset);
	
	sprintf(cols, " id, pid, uid, gid, mode, name, remark, substring(intime from '[^.]*') as intime, "
			" substring(uptime from '[^.]*') as uptime ");
	mdb_exec(conn, NULL, "SELECT %s FROM fileinfo ORDER BY id LIMIT %d OFFSET %d;",
			 NULL, cols, count, offset);
	ret = mdb_set_rows(hdf, conn, cols, "limits");
	if (ret != MDB_ERR_NONE) {
		mtc_err("get limits error %s", mdb_get_errmsg(conn));
		return RET_RBTOP_SELECTE;
	}
	HDF *res = hdf_get_obj(hdf, PRE_OUTPUT".limits.0");
	int uin = 0;
	member_t *mb;
	while (res != NULL) {
		uin = hdf_get_int_value(res, "uid", 0);
		ret = member_get_info(conn, uin, &mb);
		if (ret != RET_RBTOP_OK) {
			mtc_warn("get user %d info failure", uin);
			res = hdf_obj_next(res);
			continue;
		}
		hdf_set_value(res, "uname", mb->uname);
		member_del(mb);
		res = hdf_obj_next(res);
	}
	
	return RET_RBTOP_OK;
}

void limit_translate_mode(HDF *hdf)
{
	int mode;
	HDF *res = hdf_get_obj(hdf, PRE_OUTPUT".limits.0");

	while (res != NULL) {
		mode = hdf_get_int_value(res, "mode", 0);
		if ((mode & FILE_MASK_NOR) == FILE_MODE_NOR) {
			hdf_set_value(res, "_type", "文件");
		} else if ((mode & FILE_MODE_DIR) == FILE_MODE_DIR) {
			hdf_set_value(res, "_type", "目录");
		} else if ((mode & FILE_MODE_LINK) == FILE_MODE_LINK) {
			hdf_set_value(res, "_type", "链接");
		} else {
			hdf_set_value(res, "_type", "其他");
		}
		hdf_set_int_value(res, "_ownerp", PMS_OWNER(mode)&LMT_MASK);
		hdf_set_int_value(res, "_groupp", PMS_GROUP(mode)&LMT_MASK);
		hdf_set_int_value(res, "_otherp", PMS_OTHER(mode)&LMT_MASK);
		res = hdf_obj_next(res);
	}
}

int limit_modify(HDF *hdf, mdb_conn *conn)
{
	PRE_DBOP(hdf, conn);

	int id = hdf_get_int_value(hdf, PRE_QUERY".id", 0);
	char *area = hdf_get_value(hdf, PRE_QUERY".area", NULL);
	int unit = hdf_get_int_value(hdf, PRE_QUERY".unit", 0);
	char *enable = hdf_get_value(hdf, PRE_QUERY".enable", NULL);

	int mode = 0;
	int afrow = 0;
	int ret;
	
	if (id == 0 || area == NULL || unit <= 0 || unit > 8 || enable == NULL) {
		mtc_err("input error id: %d area:%s unit:%d enable:%s", id, area, unit, enable);
		return RET_RBTOP_INPUTE;
	}
	mdb_exec(conn, NULL, "SELECT mode FROM fileinfo WHERE id=%d;", NULL, id);
	ret = mdb_get(conn, "i", &mode);
	if (ret != MDB_ERR_NONE) {
		mtc_err("get %d mode failure", id);
		return RET_RBTOP_SELECTE;
	}
	
	if (!strcmp(area, "_ownerp")) {
		unit = unit << 4;
	} else if (!strcmp(area, "_groupp")) {
		unit = unit << 8;
	} else if (!strcmp(area, "_otherp")) {
		unit = unit << 12;
	} else {
		mtc_err("area %s error");
		return RET_RBTOP_INPUTE;
	}

	if (!strcmp(enable, "true")) {
		mode = mode | unit;
	} else {
		mode = mode & ~unit;
	}

	ret = mdb_exec(conn, &afrow, "UPDATE fileinfo set mode=%d WHERE id=%d;", NULL, mode, id);
	if (ret != RET_RBTOP_OK || afrow == 0) {
		mtc_err("update %d mode %d failure %s. affect %d rows.",
				id, mode, mdb_get_errmsg(conn), afrow);
		return RET_RBTOP_UPDATEE;
	}
	file_refresh_info(conn, id, NULL, 0);
	return RET_RBTOP_OK;
}

int limit_add(HDF *hdf, mdb_conn *conn)
{
	PRE_DBOP(hdf, conn);

	char *name = hdf_get_value(hdf, PRE_QUERY".name", NULL);
	char *remark = hdf_get_value(hdf, PRE_QUERY".remark", NULL);
	int pid = hdf_get_int_value(hdf, PRE_QUERY".pid", 0);
	int mode = hdf_get_int_value(hdf, PRE_QUERY".mode", 0);
	int uid = hdf_get_int_value(hdf, PRE_COOKIE".uin", 0);
	int gid = hdf_get_int_value(hdf, PRE_QUERY".gid", 0);

	int ret;

	if (name == NULL || remark == NULL || UIN_ILLEGAL(uid)) {
		mtc_err("input err %s %s %d", name, remark, uid);
		return RET_RBTOP_INPUTE;
	}
	char keycol[LEN_SM];
	snprintf(keycol, sizeof(keycol), "pid=%d AND name='%s'", pid, name);
	if (mmisc_get_count(conn, "fileinfo", keycol) > 0) {
		mtc_err("%d %s already exist", pid, name);
		return RET_RBTOP_EXISTE;
	}
	ret = mdb_exec(conn, NULL, "INSERT INTO fileinfo (pid, uid, gid, mode, name, remark) "
				   " VALUES (%d, %d, %d, %d, $1, $2)",
				   "ss", pid, uid, gid, mode, name, remark);
	if (ret != MDB_ERR_NONE) {
		mtc_err("add limit err %s", mdb_get_errmsg(conn));
		return RET_RBTOP_INSERTE;
	}
	
	file_t *fl;
	ret = file_get_info(conn, 0, name, pid, &fl);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get file %d %s failure", pid, name);
		return RET_RBTOP_SELECTE;
	}
	file_store_in_hdf(fl, "limits.0", hdf);
	limit_translate_mode(hdf);
	hdf_set_copy(hdf, PRE_OUTPUT".limits.0.uname", PRE_COOKIE".uname");
	file_del(fl);
	
	return RET_RBTOP_OK;
}

int limit_delete(HDF *hdf, mdb_conn *conn)
{
	PRE_DBOP(hdf, conn);

	int id = hdf_get_int_value(hdf, PRE_QUERY".id", 0);

	int ret;

	if (id == 0) {
		mtc_err("input error %d", id);
		return RET_RBTOP_INPUTE;
	}
	ret = mdb_exec(conn, NULL, "DELETE FROM fileinfo WHERE id=%d;", NULL, id);
	if (ret != MDB_ERR_NONE) {
		mtc_err("delete limit failure %s", mdb_get_errmsg(conn));
		return RET_RBTOP_DELETEE;
	}
	file_refresh_info(conn, id, NULL, 0);
	return RET_RBTOP_OK;
}
