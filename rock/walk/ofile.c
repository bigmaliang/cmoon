#include "mheads.h"
#include "lheads.h"
#include "ofile.h"
#include "member.h"

int file_check_user_power(CGI *cgi, mdb_conn *conn, file_t *file, int access)
{
	int ret;
	member_t *member;
	
	if ((PMS_OTHER(file->mode) & access) == 1) return 0;
	
	int uin = hdf_get_int_value(cgi->hdf, PRE_COOKIE".uin", -1);
	char *musn = hdf_get_value(cgi->hdf, PRE_COOKIE".musn", NULL);
	if (!member_has_login(conn, uin, musn)) {
		mtc_noise("user %d not login", uin);
		return RET_RBTOP_NOTLOGIN;
	}

	ret = member_get_info(conn, uin, &member);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get %d member info failure", uin);
		return ret;
	}

	if (member_is_owner(member, file->uid) && (PMS_OWNER(file->mode)&access) == 1) {
		member_del(member);
		return RET_RBTOP_OK;
	}

	if (member_in_group(member, file->gid) && (PMS_GROUP(file->mode)&access) == 1) {
		member_del(member);
		return RET_RBTOP_OK;
	}

#if 0
	if (member_is_friend(member, file->uid) && (PMS_FRIEND(file->mode)&access) == 1) {
		member_del(member);
		return RET_RBTOP_OK;
	}
#endif

	if (member_is_root(uin)) {
		member_del(member);
		return RET_RBTOP_OK;
	}

	member_del(member);
	return RET_RBTOP_LIMITE;
}

int file_get_info(mdb_conn *conn, int id, char *url, int pid, file_t **file)
{
	char mmckey[LEN_MMC_KEY];
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
			mdb_exec(conn, NULL, "SELECT id, pid, uid, gid, mode, reqtype, name, remark, uri, "
					 " dataer, render, substring(intime from '[^.]*') as intime, "
					 " substring(uptime from '[^.]*') as uptime "
					 " FROM fileinfo WHERE pid=%d AND name=$1;",
					 "s", pid, url);
		} else {
			mdb_exec(conn, NULL, "SELECT id, pid, uid, gid, mode, reqtype, name, remark, uri "
					 " dataer, render, substring(intime from '[^.]*') as intime, "
					 " substring(uptime from '[^.]*') as uptime "
					 " FROM fileinfo WHERE id=%d;",
					 NULL, id);
		}
		ret = mdb_get(conn, "iiiiiiSSSSSSS", &(fl->id), &(fl->pid), &(fl->uid),
					  &(fl->gid), &(fl->mode), &(fl->reqtype), &(fl->name), &(fl->remark), &(fl->uri),
					  &(fl->dataer), &(fl->render), &(fl->intime), &(fl->uptime));
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

	int pid = 0;
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
	char *buf;
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
		mdb_exec(conn, NULL, "SELECT id, pid, uid, gid, mode, reqtype, name, remark, uri, "
				 " dataer, render, substring(intime from '[^.]*') as intime, "
				 " substring(uptime from '[^.]*') as uptime "
				 " FROM fileinfo WHERE uri='%s';",
				 NULL, uri);
		ret = mdb_get(conn, "iiiiiiSSSSSSS", &(fl->id), &(fl->pid), &(fl->uid),
					  &(fl->gid), &(fl->mode), &(fl->reqtype), &(fl->name), &(fl->remark), &(fl->uri),
					  &(fl->dataer), &(fl->render), &(fl->intime), &(fl->uptime));
		if (ret != MDB_ERR_NONE) {
			mtc_err("get %s info failure from db %s", uri, mdb_get_errmsg(conn));
			if (ret == MDB_ERR_NORESULT)
				return RET_RBTOP_NEXIST;
			return RET_RBTOP_SELECTE;
		} else {
			file_pack(fl, &buf, &datalen);
			mmc_storef(MMC_OP_SET, (void*)buf, datalen, ONE_HOUR, 0, PRE_MMC_FILE".%s", uri);
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

void file_refresh_info(mdb_conn *conn, int id, char *url, int pid)
{
	file_t *fl;
	int ret;

	ret = file_get_info(conn, id, url, pid, &fl);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get info failure %d %s %d", id, url, pid);
		return;
	}

	char mmckey[LEN_MMC_KEY];
	snprintf(mmckey, LEN_MMC_KEY, "%s.%d.%s", PRE_MMC_FILE, fl->pid, fl->name);
	mmc_delete(mmckey, 0);
	snprintf(mmckey, LEN_MMC_KEY, "%s.%d", PRE_MMC_FILE, fl->id);
	mmc_delete(mmckey, 0);

	file_del(fl);
}
