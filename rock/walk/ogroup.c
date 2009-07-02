#include "mheads.h"
#include "lheads.h"
#include "ofile.h"
#include "omember.h"

#define GROUP_QUERY_COL " uid, gid, mode, status "
#define GROUP_QUERY_RAW(conn, condition, sfmt, ...)						\
	mdb_exec(conn, NULL, "SELECT "GROUP_QUERY_COL" FROM groupinfo WHERE %s;", \
			 sfmt, condition, ##__VA_ARGS__)
#define GROUP_GET_RAW(conn, gn)									\
	mdb_get(conn, "iiii", &(gn->uid), &(gn->gid), &(gn->mode), &(gn->status))

int group_get_node(mdb_conn *conn, int uid, int gid, ULIST *node)
{
	if (node == NULL || conn == NULL) return RET_RBTOP_INPUTE;
	
	gnode_t *gn;
	gn = gnode_new();
	if (gn == NULL)
		return RET_RBTOP_MEMALLOCE;

	if (!UIN_ILLEGAL(uid) && !GID_ILLEGAL(gid))
		GROUP_QUERY_RAW(conn, "uid=$1 AND gid=$2", "ii", uid, gid);
	else if (GID_ILLEGAL(gid))
		GROUP_QUERY_RAW(conn, "uid=$1", "i", uid);
	else if (UIN_ILLEGAL(uid))
		GROUP_QUERY_RAW(conn, "gid=$1", "i", gid);
	else {
		group_del(gn);
		return RET_RBTOP_INPUTE;
	}
		
	while (GROUP_GET_RAW(conn, gn) != MDB_ERR_NONE) {
		uListAppend(node, (void*)gn);
		gn = gnode_new();
	}
	gnode_del(gn);

	return RET_RBTOP_OK;
}

int group_get_info(mdb_conn *conn, int gid, group_t **group)
{
	if (gid < 0 || group == NULL)
		return RET_RBTOP_INPUTE;

	group_t *gp;
	char *buf;
	size_t datalen;
	int ret;

	buf = mmc_getf(&datalen, 0, PRE_MMC_GROUP".%d", gid);
	if (buf == NULL || datalen < sizeof(group_t)) {
		if (mdb_get_errcode(conn) != MDB_ERR_NONE) {
			mtc_err("db err %s", mdb_get_errmsg(conn));
			return RET_RBTOP_DBNINIT;
		}
		gp = group_new();
		if (gp == NULL) return RET_RBTOP_MEMALLOCE;
		gp->gid = gid;
		
		ret = group_get_node(conn, -1, gid, gp->node);
		if (ret != RET_RBTOP_OK) {
			mtc_err("get gnode failure");
			group_del(gp);
			return ret;
		}
		
		group_pack(gp, &buf, &datalen);
		mmc_storef(MMC_OP_SET, buf, datalen, ONE_DAY, 0, PRE_MMC_GROUP".%d", gid);
	} else {
		ret = group_unpack(buf, datalen, &gp);
		if (ret != RET_RBTOP_OK) {
			mtc_err("assembly group from mmc error %d", ret);
			return RET_RBTOP_MMCERR;
		}
	}
	if (buf != NULL) free(buf);
	*group = gp;

	return RET_RBTOP_OK;
}

int group_get_groups(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	PRE_DBOP(hdf, conn);

	ULIST *gids;
	file_t *fl;
	group_t *gp;
	char tok[LEN_ST];
	int id, *gid, ret, cnt = 0;

	ret = member_has_login(hdf, conn, ses);
	if (ret != RET_RBTOP_OK) {
		mtc_warn("%d check login failure %d", ses->member->uin, ret);
		return ret;
	}

	ret = member_get_group(ses->member, GROUP_MODE_JUNIOR, &gids);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get %s %d failure", ses->member->gmodes, GROUP_MODE_JUNIOR);
		return ret;
	}

	MLIST_ITERATE(gids, gid) {
		ret = file_get_info_by_id(conn, *gid, NULL, -1, &fl);
		if (ret != RET_RBTOP_OK) {
			mtc_err("get %d info failure");
			continue;
		}
		sprintf(tok, "%s.groups.%d", PRE_OUTPUT, cnt++);
		file_store_in_hdf(fl, tok, hdf);
		file_del(fl);
	}
	hdf_set_int_value(hdf, PRE_OUTPUT".ttnum", cnt);
	uListDestroy(&gids, ULIST_FREE);

	HDF *node = hdf_get_obj(hdf, PRE_OUTPUT".groups.0");
	while (node != NULL) {
		id = hdf_get_int_value(node, "gid", -1);
		
		if (member_in_group(ses->member, id, GROUP_MODE_ADM)) {
			hdf_set_value(node, "amadmin", "1");
		}
		
		ret = group_get_info(conn, id, &gp);
		if (ret != RET_RBTOP_OK) {
			mtc_warn("get member for group %d failure", gid);
			continue;
		}
		group_store_in_hdf(gp, "groupinfo", node);
		node = hdf_obj_next(node);
	}
	
	return RET_RBTOP_OK;
}

int group_add_member(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	PRE_DBOP(hdf, conn);

	int uid = hdf_get_int_value(hdf, PRE_QUERY".uid", 0);
	int gid = hdf_get_int_value(hdf, PRE_QUERY".gid", 0);
	int mode = hdf_get_int_value(hdf, PRE_QUERY".mode", 0);
	int status = hdf_get_int_value(hdf, PRE_QUERY".status", 0);
	int ret;

	if (UIN_ILLEGAL(uid) || GID_ILLEGAL(gid)) {
		mtc_err("uid %d gid %d illegal", uid, gid);
		return RET_RBTOP_INPUTE;
	}

	ret = member_has_login(hdf, conn, ses);
	if (ret != RET_RBTOP_OK) {
		mtc_warn("%d check login failure %d", ses->member->uin, ret);
		return ret;
	}

	if (!member_in_group(ses->member, gid, GROUP_MODE_ADM)) {
		mtc_err("%d attemped  add group member for %d, limited",
				ses->member->uin, gid);
		return RET_RBTOP_LIMITE;
	}

	member_t *mb;
	ret = member_get_info(conn, uid, &mb);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get %d info failure", uid);
		return ret;
	}
	if (member_in_group(mb, gid, GROUP_MODE_JOIN)) {
		ret = RET_RBTOP_EXISTE;
	}
	member_del(mb);
	
}
