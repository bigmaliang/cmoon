#include "mheads.h"
#include "lheads.h"
#include "ofile.h"
#include "omember.h"

#define FILE_QUERY_COL	" id, pid, uid, gid, mode, reqtype, lmttype, name, remark, uri, " \
	" dataer, render, substring(intime from '[^ ]*') as intime, "		\
	" substring(uptime from '[^ ]*') as uptime "

#define FILE_QUERY_RAW(conn, condition, sfmt, ...)						\
	mdb_exec(conn, NULL, "SELECT "FILE_QUERY_COL" FROM fileinfo WHERE %s;", \
			 sfmt, condition, ##__VA_ARGS__)

#define FILE_GET_RAW(conn, fl)											\
	mdb_get(conn, "iiiiiiiSSSSSSS", &(fl->id), &(fl->pid), &(fl->uid), &(fl->gid), \
			&(fl->mode), &(fl->reqtype), &(fl->lmttype), &(fl->name), &(fl->remark), \
			&(fl->uri),	&(fl->dataer), &(fl->render), &(fl->intime), &(fl->uptime))


int file_check_user_power(HDF *hdf, mdb_conn *conn, session_t *ses,
						  file_t *file, int access)
{
	int ret;
	
	if ((PMS_OTHER(file->mode) & access) == 1) return 0;

	ret = member_has_login(hdf, conn, ses);
	if (ret != RET_RBTOP_OK) {
		mtc_noise("not login");
		return RET_RBTOP_NOTLOGIN;
	}

	if (member_is_owner(ses->member, file->uid) &&
		(PMS_OWNER(file->mode)&access) == 1) {
		return RET_RBTOP_OK;
	}

	if (member_in_group(ses->member, file->gid) &&
		(PMS_GROUP(file->mode)&access) == 1) {
		return RET_RBTOP_OK;
	}

#if 0
	if (member_is_friend(ses->member, file->uid) &&
		(PMS_FRIEND(file->mode)&access) == 1) {
		return RET_RBTOP_OK;
	}
#endif

	if (member_is_root(ses->member)) {
		return RET_RBTOP_OK;
	}

	return RET_RBTOP_LIMITE;
}

int file_get_info(mdb_conn *conn, int id, char *url, int pid, file_t **file)
{
	char mmckey[LEN_MMC_KEY], tok[LEN_MD];
	file_t *fl;
	size_t datalen;
	char *buf;
	int ret;
	
	if (id < 0 && (url == NULL || pid < 0))	return RET_RBTOP_INPUTE;

	if (id <= 0)
		snprintf(mmckey, LEN_MMC_KEY, "%s.%d.%s", PRE_MMC_FILE, pid, url);
	else
		snprintf(mmckey, LEN_MMC_KEY, "%s.%d", PRE_MMC_FILE, id);
	buf = mmc_get(mmckey, &datalen, 0);
	if (buf == NULL || datalen < sizeof(file_t)) {
		if (buf != NULL && datalen < sizeof(file_t)) {
			mtc_warn("get %d %d.%s info error from mmc %d",
					 id, pid, url, datalen);
		}
		if (mdb_get_errcode(conn) != MDB_ERR_NONE) {
			mtc_err("conn err %s", mdb_get_errmsg(conn));
			return RET_RBTOP_INPUTE;
		}
		fl = file_new();
		if (fl == NULL) return RET_RBTOP_MEMALLOCE;
		if (id <= 0) {
			snprintf(tok, sizeof(tok), "pid=%d AND name=$1", pid);
			FILE_QUERY_RAW(conn, tok, "s", url);
		} else {
			snprintf(tok, sizeof(tok), "id=%d", id);
			FILE_QUERY_RAW(conn, tok, NULL);
		}
		ret = FILE_GET_RAW(conn, fl);
		if (ret != MDB_ERR_NONE) {
			mtc_err("get %d %d.%s info failure from db %s",
					id, pid, url, mdb_get_errmsg(conn));
			if (ret == MDB_ERR_NORESULT)
				return RET_RBTOP_NEXIST;
			return RET_RBTOP_SELECTE;
		} else {
			file_pack(fl, &buf, &datalen);
			mmc_store(MMC_OP_SET, mmckey, (void*)buf, datalen, ONE_HOUR, 0);
		}
	} else {
		ret = file_unpack(buf, datalen, &fl);
		if (ret != 0) {
			mtc_err("assembly file from mmc error");
			return RET_RBTOP_MMCERR;
		}
	}
	free(buf);
	*file = fl;
	return RET_RBTOP_OK;
}
/*
 * get fileset's info
 * inp: urls, url list you want to get
 * out: files, file list with file_t elements. don't forget free
 */
int file_get_infos(mdb_conn *conn, ULIST *urls, ULIST **files, int *noksn)
{
	int listlen;
	char *url;
	file_t *file;
	NEOERR *err;
	int ret;

	listlen = uListLength(urls);
	if (listlen <= 0 || url == NULL || files == NULL) {
		return RET_RBTOP_INPUTE;
	}

	err = uListInit(files, 0, 0);
	RETURN_V_NOK(err, RET_RBTOP_MEMALLOCE);

	int pid = 1;
	int i;
	for (i = 0; i < listlen; i++) {
		err = uListGet(urls, i, (void**)&url);
		RETURN_V_NOK(err, RET_RBTOP_GETLISTE);
		ret = file_get_info(conn, 0, url, pid, &file);
		if (ret != RET_RBTOP_OK) {
			mtc_warn("can't get file info for %s", url);
			*noksn = i;
			return RET_RBTOP_GETLISTE;
		} else {
			pid = file->id;
			uListAppend(*files, file);
		}
	}
	*noksn = -1;
	return RET_RBTOP_OK;
}

int file_get_info_uri(mdb_conn *conn, char *uri, file_t **file)
{
	file_t *fl;
	size_t datalen;
	char *buf, tok[LEN_MD];
	int ret;
	
	if (uri == NULL) return RET_RBTOP_INPUTE;

	buf = mmc_getf(&datalen, 0, PRE_MMC_FILE".%s", uri);
	if (buf == NULL || datalen < sizeof(file_t)) {
		if (buf != NULL && datalen < sizeof(file_t)) {
			mtc_warn("get %s info error from mmc %d", uri, datalen);
		}
		if (mdb_get_errcode(conn) != MDB_ERR_NONE) {
			mtc_err("conn err %s", mdb_get_errmsg(conn));
			return RET_RBTOP_INPUTE;
		}
		fl = file_new();
		if (fl == NULL) return RET_RBTOP_MEMALLOCE;
		snprintf(tok, sizeof(tok), "uri='%s'", uri);
		FILE_QUERY_RAW(conn, tok, NULL);
		ret = FILE_GET_RAW(conn, fl);
		if (ret != MDB_ERR_NONE) {
			mtc_err("get %s info failure from db %s", uri, mdb_get_errmsg(conn));
			if (ret == MDB_ERR_NORESULT)
				return RET_RBTOP_NEXIST;
			return RET_RBTOP_SELECTE;
		} else {
			file_pack(fl, &buf, &datalen);
			mmc_storef(MMC_OP_SET, (void*)buf, datalen, ONE_HOUR, 0,
					   PRE_MMC_FILE".%s", uri);
		}
	} else {
		ret = file_unpack(buf, datalen, &fl);
		if (ret != 0) {
			mtc_err("assembly file from mmc error");
			return RET_RBTOP_MMCERR;
		}
	}
	free(buf);
	*file = fl;
	return RET_RBTOP_OK;
}

void file_refresh_me(file_t *fl)
{
	mmc_deletef(0, PRE_MMC_FILE".%d.%s", fl->pid, fl->name);
	mmc_deletef(0, PRE_MMC_FILE".%d", fl->id);
}

void file_refresh_info(mdb_conn *conn, int id, char *url, int pid)
{
	file_t *fl;
	int ret;

	ret = file_get_info(conn, id, url, pid, &fl);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get info failure %d %s %d", id, url, pid);
		return;
	}
	file_refresh_me(fl);
	file_del(fl);
}


int file_get_files(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	PRE_DBOP(hdf, conn);

	int ret, cnt = 0;
	char tok[LEN_MD];
	int count, offset, uin, pid, limit;
	member_t *mb;
	file_t *fl;

	pid = hdf_get_int_value(hdf, PRE_QUERY".pid", 1);
	limit = hdf_get_int_value(hdf, PRE_QUERY".limit", 0);

	sprintf(tok, "pid=%d", pid);
	mmisc_set_count(hdf, conn, "fileinfo", tok);
	mmisc_get_offset(hdf, &count, &offset);

	if (member_has_login(hdf, conn, ses) != RET_RBTOP_OK) {
		return RET_RBTOP_NOTLOGIN;
	}

	fl = file_new();
	if (fl == NULL) {
		mtc_err("file_new failure");
		return RET_RBTOP_MEMALLOCE;
	}
	
	snprintf(tok, sizeof(tok), "pid=%d ORDER BY id LIMIT %d OFFSET %d",
			 pid, count, offset);
	FILE_QUERY_RAW(conn, tok, NULL);
	while (FILE_GET_RAW(conn, fl) == MDB_ERR_NONE) {
		ret = file_check_user_power(hdf, conn, ses, fl, limit);
		if (ret == RET_RBTOP_OK) {
			sprintf(tok, "files.%d", cnt++);
			file_store_in_hdf(fl, tok, hdf);
		} else {
			mtc_noise("user %d has no %s's %d limit",
					  ses->member->uin, fl->uri, limit);
		}
		file_reset(fl);
	}
	
	HDF *res = hdf_get_obj(hdf, PRE_OUTPUT".files.0");
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

void file_translate_mode(HDF *hdf)
{
	int mode;
	HDF *res = hdf_get_obj(hdf, PRE_OUTPUT".files.0");

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

int file_modify(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	PRE_DBOP(hdf, conn);

	file_t *fl;
	
	int id = hdf_get_int_value(hdf, PRE_QUERY".id", 0);
	char *area = hdf_get_value(hdf, PRE_QUERY".area", NULL);
	int unit = hdf_get_int_value(hdf, PRE_QUERY".unit", 0);
	char *enable = hdf_get_value(hdf, PRE_QUERY".enable", NULL);

	int mode = 0;
	int afrow = 0;
	int ret;
	
	if (id == 0 || area == NULL || unit <= 0 || unit > 8 || enable == NULL) {
		mtc_err("input error id: %d area:%s unit:%d enable:%s",
				id, area, unit, enable);
		return RET_RBTOP_INPUTE;
	}
	ret = file_get_info(conn, id, NULL, -1, &fl);
	if (ret != RET_RBTOP_OK) return ret;
	ret = file_check_user_power(hdf, conn, ses, fl, LMT_MOD);
	if (ret != RET_RBTOP_OK) {
		mtc_warn("%d attemped modify %s, limited", ses->member->uin, fl->uri);
		goto done;
	}
	
	mode = fl->mode;
	if (!strcmp(area, "_ownerp")) {
		unit = unit << 4;
	} else if (!strcmp(area, "_groupp")) {
		unit = unit << 8;
	} else if (!strcmp(area, "_otherp")) {
		unit = unit << 12;
	} else {
		mtc_err("area %s error");
		ret = RET_RBTOP_INPUTE;
		goto done;
	}

	if (!strcmp(enable, "true")) {
		mode = mode | unit;
	} else {
		mode = mode & ~unit;
	}
	
	ret = MDATA_SET(conn, EVT_PLUGIN_SYS, &afrow, FLAGS_NONE,
					"UPDATE fileinfo set mode=%d WHERE id=%d;",	NULL, mode, id);
	if (ret != RET_RBTOP_OK || afrow == 0) {
		mtc_err("update %d mode %d failure %s. affect %d rows.",
				id, mode, mdb_get_errmsg(conn), afrow);
		goto done;
	}
	file_refresh_me(fl);

 done:
	file_del(fl);
	return ret;
}

int file_add(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	PRE_DBOP(hdf, conn);

	file_t *fl;
	char *name = hdf_get_value(hdf, PRE_QUERY".name", NULL);
	char *remark = hdf_get_value(hdf, PRE_QUERY".remark", NULL);
	int pid = hdf_get_int_value(hdf, PRE_QUERY".pid", 1);
	int mode = hdf_get_int_value(hdf, PRE_QUERY".mode", 0);
	int uid = hdf_get_int_value(hdf, PRE_COOKIE".uin", 0);
	int gid = hdf_get_int_value(hdf, PRE_QUERY".gid", 0);

	int ret;

	if (name == NULL || remark == NULL || UIN_ILLEGAL(uid)) {
		mtc_err("input err %s %s %d", name, remark, uid);
		return RET_RBTOP_INPUTE;
	}

	ret = file_get_info(conn, pid, NULL, -1, &fl);
	if (ret != RET_RBTOP_OK) return ret;
	ret = file_check_user_power(hdf, conn, ses, fl, LMT_APPEND);
	if (ret != RET_RBTOP_OK) {
		mtc_warn("%d attemped add file to %s, limited", ses->member->uin, fl->uri);
		goto done;
	}

	char keycol[LEN_SM];
	/* TODO sql hole */
	snprintf(keycol, sizeof(keycol), "pid=%d AND name='%s'", pid, name);
	if (mmisc_get_count(conn, "fileinfo", keycol) > 0) {
		mtc_err("%d %s already exist", pid, name);
		ret = RET_RBTOP_EXISTE;
		goto done;
	}
	ret = MDATA_SET(conn, EVT_PLUGIN_SYS, NULL, FLAGS_NONE, "INSERT INTO fileinfo "
					" (pid, uid, gid, mode, name, remark) VALUES "
					" (%d, %d, %d, %d, $1, $2)", "ss",
					pid, uid, gid, mode, name, remark);
	if (ret != MDB_ERR_NONE) {
		mtc_err("add file err %s", mdb_get_errmsg(conn));
		goto done;
	}

	file_del(fl);
	ret = file_get_info(conn, 0, name, pid, &fl);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get file %d %s failure", pid, name);
		goto done;
	}
	file_store_in_hdf(fl, "files.0", hdf);
	file_translate_mode(hdf);
	hdf_set_copy(hdf, PRE_OUTPUT".files.0.uname", PRE_COOKIE".uname");

 done:
	file_del(fl);
	return ret;
}

int file_delete(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	PRE_DBOP(hdf, conn);

	file_t *fl;

	int id = hdf_get_int_value(hdf, PRE_QUERY".id", 0);

	int ret;

	if (id == 0) {
		mtc_err("input error %d", id);
		return RET_RBTOP_INPUTE;
	}
	ret = file_get_info(conn, id, NULL, -1, &fl);
	if (ret != RET_RBTOP_OK) return ret;
	ret = file_check_user_power(hdf, conn, ses, fl, LMT_DEL);
	if (ret != RET_RBTOP_OK) {
		mtc_warn("%d attemped del file %s, limited", ses->member->uin, fl->uri);
		goto done;
	}

	ret = MDATA_SET(conn, EVT_PLUGIN_SYS, NULL, FLAGS_NONE,
					"DELETE FROM fileinfo WHERE id=%d;", NULL, id);
	if (ret != MDB_ERR_NONE) {
		mtc_err("delete file failure %s", mdb_get_errmsg(conn));
		return RET_RBTOP_DELETEE;
	}

	file_refresh_me(fl);
	
 done:
	file_del(fl);
	return ret;
}

int file_get_action(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	char tok[256];
	
	if (member_has_login(hdf, conn, ses) != RET_RBTOP_OK) {
		return RET_RBTOP_NOTLOGIN;
	}

	memset(tok, 0x0, sizeof(tok));
	
	if (member_is_root(ses->member)) {
		sprintf(tok, "lmttype >= %d", LMT_TYPE_START);
	} else if (member_has_gmode(ses->member, GROUP_MODE_OWN)) {
		sprintf(tok, "lmttype >= %d AND lmttype < %d",
				LMT_TYPE_START, LMT_TYPE_ROOT);
		FILE_QUERY_RAW(conn, tok, NULL);
	} else if (member_has_gmode(ses->member, GROUP_MODE_ADM)) {
		sprintf(tok, "lmttype >= %d AND lmttype < %d",
				LMT_TYPE_START, LMT_TYPE_GOWN);
	} else if (member_has_gmode(ses->member, GROUP_MODE_JOIN)) {
		sprintf(tok, "lmttype >= %d AND lmttype < %d",
				LMT_TYPE_START, LMT_TYPE_GADM);
	} else {
		sprintf(tok, "lmttype >= %d AND lmttype < %d",
				LMT_TYPE_START, LMT_TYPE_GJOIN);
	}

	FILE_QUERY_RAW(conn, tok, NULL);
	mdb_set_rows(hdf, conn, FILE_QUERY_COL, "actions");

	HDF *node = hdf_get_obj(hdf, PRE_OUTPUT".actions.0");
	char href[_POSIX_PATH_MAX];
	while (node != NULL) {
		memset(href, 0x0, sizeof(href));
		hdf_set_copy(node, "name", "remark");
		hdf_set_copy(node, "href", "uri");
		if (hdf_get_int_value(node, "reqtype", 0)
			== CGI_REQ_AJAX) {
			strncpy(href, hdf_get_value(node, "uri", "/index"),
					sizeof(href));
			strcat(href, ".html");
			hdf_set_value(node, "href", href);
		}
		node = hdf_obj_next(node);
	}

	return RET_RBTOP_OK;
}

int file_get_nav(HDF *hdf, mdb_conn *conn)
{
	return RET_RBTOP_OK;
}
