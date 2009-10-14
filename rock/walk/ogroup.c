#include "mheads.h"
#include "lheads.h"
#include "ofile.h"
#include "omember.h"

#define GROUP_QUERY_COL " uid, gid, mode, status, to_char(intime, 'YYYY-MM-DD') " \
    " as intime, to_char(uptime, 'YYYY-MM-DD') as uptime "
#define GROUP_GET_RAW(conn, gn)									\
	mdb_get(conn, "iiiiSS", &(gn->uid), &(gn->gid), &(gn->mode), &(gn->status), \
			&(gn->intime), &(gn->uptime))

/*
 * group_node, an abstract/concept data, not invisible by application direct.
 * so, cache is done by my wrapper
 */
int group_get_node(mdb_conn *conn, int uid, int gid, ULIST *node)
{
	if (node == NULL || conn == NULL) return RET_RBTOP_INPUTE;
	
	gnode_t *gn;
	gn = gnode_new();
	if (gn == NULL)
		return RET_RBTOP_MEMALLOCE;

	if (!UIN_ILLEGAL(uid) && !GID_ILLEGAL(gid))
		LDB_QUERY_RAW(conn, "groupinfo", GROUP_QUERY_COL,
                        "uid=%d AND gid=%d", NULL, uid, gid);
	else if (GID_ILLEGAL(gid))
		LDB_QUERY_RAW(conn, "groupinfo", GROUP_QUERY_COL, "uid=%d", NULL, uid);
	else if (UIN_ILLEGAL(uid))
		LDB_QUERY_RAW(conn, "groupinfo", GROUP_QUERY_COL, "gid=%d", NULL, gid);
	else {
		group_del(gn);
		return RET_RBTOP_INPUTE;
	}
		
	while (GROUP_GET_RAW(conn, gn) == MDB_ERR_NONE) {
		uListAppend(node, (void*)gn);
		gn = gnode_new();
	}
	gnode_del(gn);

	return RET_RBTOP_OK;
}

void group_refresh_info(int gid)
{
	mmc_deletef(0, PRE_MMC_GROUP".%d", gid);
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
		ret = group_unpack(buf, datalen, &gp, NULL);
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

	ULIST *gnode;
	gnode_t *node;
	file_t *fl;
	group_t *gp;
	char tok[LEN_ST];
	int id, ret, cnt = 0;

	ret = member_has_login(hdf, conn, ses);
	if (ret != RET_RBTOP_OK) {
		mtc_warn("%d check login failure %d", ses->member->uin, ret);
		return ret;
	}

	ret = member_get_group(ses->member, GROUP_MODE_JUNIOR, GROUP_STAT_OK, &gnode);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get %d %d failure", ses->member->uin, GROUP_MODE_JUNIOR);
		return ret;
	}

	MLIST_ITERATE(gnode, node) {
		ret = file_get_info_by_id(conn, node->gid, NULL, -1, &fl);
		if (ret != RET_RBTOP_OK) {
			mtc_err("get %d info failure", node->gid);
			continue;
		}
		sprintf(tok, "%s.groups.%d", PRE_OUTPUT, cnt++);
		file_item2hdf(fl, tok, hdf);
		file_del(fl);
	}
	hdf_set_int_value(hdf, PRE_OUTPUT".ttnum", cnt);
	uListDestroy(&gnode, 0);

	HDF *hdfnode = hdf_get_obj(hdf, PRE_OUTPUT".groups.0");
	while (hdfnode != NULL) {
		id = hdf_get_int_value(hdfnode, "gid", -1);
		
		if (member_in_group(ses->member, id, GROUP_MODE_ADM, GROUP_STAT_OK, NULL)) {
			hdf_set_value(hdfnode, "amadmin", "1");
		}
		
		ret = group_get_info(conn, id, &gp);
		if (ret != RET_RBTOP_OK) {
			mtc_warn("get member for group %d failure", id);
			continue;
		}
		group_item2hdf(gp, "groupinfo", hdfnode);
		hdfnode = hdf_obj_next(hdfnode);
	}
	
	return RET_RBTOP_OK;
}

int group_add_member(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	PRE_DBOP(hdf, conn);

	gnode_t *node;
	NEOERR *err;
	int uid = hdf_get_int_value(hdf, PRE_QUERY".uid", 0);
	int gid = hdf_get_int_value(hdf, PRE_QUERY".gid", 0);
	int mode = hdf_get_int_value(hdf, PRE_QUERY".mode", 0);
	int status = hdf_get_int_value(hdf, PRE_QUERY".status", 0);
	int pos, ret;

	if (UIN_ILLEGAL(uid) || GID_ILLEGAL(gid)) {
		mtc_err("uid %d gid %d illegal", uid, gid);
		return RET_RBTOP_INPUTE;
	}

	ret = member_has_login(hdf, conn, ses);
	if (ret != RET_RBTOP_OK) {
		mtc_warn("%d check login failure %d", ses->member->uin, ret);
		return ret;
	}

	if (!member_in_group(ses->member, gid, GROUP_MODE_JOIN, GROUP_STAT_OK, &pos)) {
		mtc_warn("%d attemped  add group member for %d, limited",
				ses->member->uin, gid);
		return RET_RBTOP_LIMITE;
	}
	/*
	 * root without join will return pos with -1 
	 */
	if (pos > 0) {
		err = uListGet(ses->member->gnode, pos, (void**)&node);
		RETURN_V_NOK(err, RET_RBTOP_ERROR);
		if (node->mode < mode) {
			mtc_warn("%d attemped add group %d member mode %d with mode %d, limited",
					 ses->member->uin, gid, mode, node->mode);
			return RET_RBTOP_LIMITE;
		}
	}

	member_t *mb;
	ret = member_get_info(conn, uid, &mb);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get %d info failure", uid);
		return ret;
	}

	if (member_in_group(mb, gid, GROUP_MODE_JOIN, GROUP_STAT_ALL, &pos) && pos > 0) {
		err = uListGet(mb->gnode, pos, (void**)&node);
		if (err != STATUS_OK) ret = RET_RBTOP_ERROR;
		JUMP_NOK(err, done);
		switch (node->status) {
		case GROUP_STAT_APPLY:
			if (status != GROUP_STAT_REJECT && status != GROUP_STAT_OK) {
				ret = RET_RBTOP_OPCODEE;
				goto done;
			}
			break;
		case GROUP_STAT_INVITE:
			if (status != GROUP_STAT_REJECT && status != GROUP_STAT_OK) {
				ret = RET_RBTOP_OPCODEE;
				goto done;
			}
			break;
		case GROUP_STAT_OK:
			if (mode == node->mode) {
				ret = RET_RBTOP_EXISTE;
				goto done;
			}
			break;
		default:
			ret = RET_RBTOP_OPCODEE;
			goto done;
		}
		ret = MDATA_SET(conn, EVT_PLUGIN_SYS, NULL, FLAGS_NONE,
						"UPDATE groupinfo SET status=%d, mode=%d WHERE uid=%d "
						" AND GID=%d;",	NULL, status, mode, mb->uin, gid);
	} else {
		ret = MDATA_SET(conn, EVT_PLUGIN_SYS, NULL, FLAGS_NONE,
						"INSERT INTO groupinfo (uid, gid, mode, status) VALUES "
						" (%d, %d, %d, %d);", NULL, mb->uin, gid, mode, status);
	}
	member_refresh_info(mb->uin);
	group_refresh_info(gid);
	
 done:
	member_del(mb);
	return ret;
}

int group_del_member(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	PRE_DBOP(hdf, conn);

	gnode_t *node;
	NEOERR *err;
	int uid = hdf_get_int_value(hdf, PRE_QUERY".uid", 0);
	int gid = hdf_get_int_value(hdf, PRE_QUERY".gid", 0);
	int pos, ret;

	if (UIN_ILLEGAL(uid) || GID_ILLEGAL(gid)) {
		mtc_err("uid %d gid %d illegal", uid, gid);
		return RET_RBTOP_INPUTE;
	}

	ret = member_has_login(hdf, conn, ses);
	if (ret != RET_RBTOP_OK) {
		mtc_warn("%d check login failure %d", ses->member->uin, ret);
		return ret;
	}

	if (!member_in_group(ses->member, gid, GROUP_MODE_JOIN, GROUP_STAT_OK, &pos)) {
		mtc_warn("%d attemped  add group member for %d, limited",
				ses->member->uin, gid);
		return RET_RBTOP_LIMITE;
	}
	/*
	 * root without join will return pos with -1 
	 */
	int mode_oper = 0;
	if (pos > 0) {
		err = uListGet(ses->member->gnode, pos, (void**)&node);
		RETURN_V_NOK(err, RET_RBTOP_ERROR);
		mode_oper = node->mode;
	} else {
		mode_oper = GROUP_MODE_ROOT;
	}
	
	member_t *mb;
	ret = member_get_info(conn, uid, &mb);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get %d info failure", uid);
		return ret;
	}

	if (member_in_group(mb, gid, GROUP_MODE_JOIN, GROUP_STAT_ALL, &pos) && pos > 0) {
		err = uListGet(mb->gnode, pos, (void**)&node);
		if (err != STATUS_OK) ret = RET_RBTOP_ERROR;
		JUMP_NOK(err, done);
		if (mode_oper < node->mode) {
			mtc_warn("%d attemped del group %d member mode %d with mode %d, limited",
					 ses->member->uin, gid, node->mode, mode_oper);
			ret = RET_RBTOP_LIMITE;
			goto done;
		}
		ret = MDATA_SET(conn, EVT_PLUGIN_SYS, NULL, FLAGS_NONE,
						"DELETE FROM groupinfo WHERE uid=%d AND gid=%d;",
						NULL, mb->uin, gid);
	} else {
		ret = RET_RBTOP_NEXIST;
		goto done;
	}
	member_refresh_info(mb->uin);
	group_refresh_info(gid);

 done:
	member_del(mb);
	return ret;
}
