#include "mheads.h"
#include "lheads.h"
#include "ofile.h"
#include "omember.h"

#define GROUP_QUERY_COL " uid, mode, stat "
#define GROUP_QUERY_RAW(conn, condition, sfmt, ...)						\
	mdb_exec(conn, NULL, "SELECT "GROUP_QUERY_COL" FROM groupinfo WHERE %s;", \
			 sfmt, condition, ##__VA_ARGS__)
#define GROUP_GET_RAW(conn, gn)									\
	mdb_get(conn, "iii", &(gn->uid), &(gn->mode), &(gn->stat))

int group_get_info(mdb_conn *conn, int gid, group_t **group)
{
	if (gid < 0 || group == NULL)
		return RET_RBTOP_INPUTE;

	group_t *gp;
	gnode_t *gn;
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
		gn = gnode_new();
		if (gp == NULL || gn == NULL) return RET_RBTOP_MEMALLOCE;
		gp->gid = gid;
		
		GROUP_QUERY_RAW(conn, "gid=$1", "i", gid);
		while (GROUP_GET_RAW(conn, gn) != MDB_ERR_NONE) {
			if (gn == NULL) {
				mtc_err("new gnode failure");
				continue;
			}
			uListAppend(gp->node, (void*)gn);
			gn = gnode_new();
		}
		gnode_del(gn);
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

int group_get_group(HDF *hdf, mdb_conn *conn, session_t *ses)
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
