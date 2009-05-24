#include "mheads.h"
#include "lheads.h"

#include "member.h"

void member_refresh_info(uin)
{
	char mmckey[LEN_MMC_KEY];
	sprintf(mmckey, "%s.%d", PRE_MMC_MEMBER, uin);
	mmc_delete(mmckey, 0);
}
void member_set_unameck(CGI *cgi)
{
	char *uname = hdf_get_value(cgi->hdf, PRE_QUERY".uname", "");
	if (!strcmp(uname, "")) {
		mtc_warn("can't find uname in hdf");
		return;
	}

	char *uname_esc;
	cgi_url_escape(uname, &uname_esc);
	cgi_cookie_set(cgi, "uname", uname_esc, NULL, SITE_DOMAIN, NULL, 1, 0);
	free(uname_esc);
}
void member_remember_login(CGI *cgi, mdb_conn *conn, int uin)
{
	PRE_DBOP_NRET(cgi->hdf, conn);
	
	char usertable[LEN_TB];
	char musn[LEN_CK];
	char *musn_esc;

	char *tmp = hdf_get_value(cgi->hdf, PRE_CGI".Cookie.musn", NULL);
	if (tmp == NULL) {
		sprintf(usertable, "user_%d", uin%DIV_USER_TB);
		memset(musn, 0x0, sizeof(musn));
		neo_rand_string(musn, sizeof(musn));
		cgi_url_escape(musn, &musn_esc);

		mdb_exec(conn, NULL, "UPDATE %s SET musn=$1 WHERE uin=%d",
				 "s", usertable, uin, musn_esc);
		hdf_set_value(cgi->hdf, PRE_OUTPUT".musn", musn_esc);
		member_refresh_info(uin);
		/*
		 * currently, we store a templory usn into cookie for auth
		 * just a while
		 */
		char tm[LEN_TM_GMT];
		mmisc_getdatetime(tm, sizeof(tm), "%A, %d-%b-%Y %T GMT", 60*60*3);
		cgi_cookie_set(cgi, "musn", musn_esc, NULL, SITE_DOMAIN, tm, 1, 0);
		free(musn_esc);
	}
}

int member_release_uin(HDF *hdf, mdb_conn *conn)
{
	PRE_DBOP(hdf, conn);

	int uin;
	int rows = 0;
	int ret;

	/*
	 * get a fresh number 
	 */
	mdb_exec(conn, NULL, "SELECT uin FROM %s WHERE status=%d ORDER BY RANDOM() LIMIT 1;",
			 NULL, TABLE_RLS_USER, MNUM_USER_FRESH);
	ret = mdb_get(conn, "i", &uin);
	if (ret != MDB_ERR_NONE) {
		mtc_err("get rls number failuer %s", mdb_get_errmsg(conn));
		return RET_RBTOP_SELECTE;
	} else {
		mtc_info("get rls number %d ok", uin);
	}

	/*
	 * mark released 
	 */
	ret = mdb_exec(conn, &rows, "UPDATE %s SET status=%d, uname=$1, male=$2 WHERE uin=%d AND status=%d;",
				   "si", TABLE_RLS_USER, MNUM_USER_RLSED, uin, MNUM_USER_FRESH,
				   hdf_get_value(hdf, PRE_QUERY".uname", ""),
				   hdf_get_int_value(hdf, PRE_QUERY".male", 1));
	if (ret != MDB_ERR_NONE || rows == 0) {
		mtc_err("set rls number %d failure %s. affect %d rows.",
				uin, mdb_get_errmsg(conn), rows);
		return RET_RBTOP_UPDATEE;
	}
	
	hdf_set_int_value(hdf, PRE_OUTPUT".uin", uin);
	return RET_RBTOP_OK;
}

int member_alloc_uin(HDF *hdf, mdb_conn *conn)
{
	PRE_DBOP(hdf, conn);

	int rows;
	int ret;

	int uin = hdf_get_int_value(hdf, PRE_QUERY".uin", 0);
	if (uin < MIN_USER_NUM)
		return RET_RBTOP_INPUTE;

	/*
	 * mark released 
	 */
	ret = mdb_exec(conn, &rows, "UPDATE %s SET status=%d, uname=$1, male=$2 WHERE uin=%d AND status=%d;",
				   "si", TABLE_RLS_USER, MNUM_USER_RLSED, uin, MNUM_USER_FRESH,
				   hdf_get_value(hdf, PRE_QUERY".uname", ""),
				   hdf_get_int_value(hdf, PRE_QUERY".male", 1));
	if (ret != MDB_ERR_NONE) {
		mtc_err("set alloc number %d failure %s.", uin, mdb_get_errmsg(conn));
		return RET_RBTOP_UPDATEE;
	}
	if (rows == 0) {
		mtc_warn("set alloc number %d failure. %d.", uin, rows);
		return RET_RBTOP_RELEASED;
	}

	return RET_RBTOP_OK;
}

int member_confirm_uin(HDF *hdf, mdb_conn *conn)
{
	PRE_DBOP(hdf, conn);

	char usertable[LEN_TB];
	int rows = 0;
	int uin = hdf_get_int_value(hdf, PRE_QUERY".uin", 0);
	if (uin < MIN_USER_NUM)
		return RET_RBTOP_INPUTE;
	int ret;

	/*
	 * db process 
	 */
	sprintf(usertable, "user_%d", uin%DIV_USER_TB);
	mdb_begin(conn);
	mdb_exec(conn, NULL, "UPDATE %s SET status=%d, usn=$1, email=$2 WHERE uin=%d AND status=%d;", "ss",
			 TABLE_RLS_USER, MNUM_USER_CFMED, uin, MNUM_USER_RLSED,
			 hdf_get_value(hdf, PRE_QUERY".usn", ""),
			 hdf_get_value(hdf, PRE_QUERY".email", ""));
	mdb_exec(conn, &rows, "INSERT INTO %s SELECT * FROM %s WHERE uin=%d;", NULL,
			 usertable, TABLE_RLS_USER, uin);
	mdb_exec(conn, NULL, "UPDATE %s SET intime=now() WHERE uin=%d;", NULL, usertable, uin);
	ret = mdb_finish(conn);
	if (ret != MDB_ERR_NONE || rows == 0) {
		mtc_err("confirm number %d failure %s. affect %d rows", uin, mdb_get_errmsg(conn), rows);
		return RET_RBTOP_UPDATEE;
	}

	/*
	 * prepare uname for later use 
	 */
	char *uname;
	mdb_exec(conn, NULL, "SELECT uname FROM %s WHERE uin=%d;", NULL, usertable, uin);
	ret = mdb_get(conn, "s", &uname);
	if (ret != MDB_ERR_NONE) {
		mtc_err("get %d's uname failuer %s", uin, mdb_get_errmsg(conn));
		return RET_RBTOP_SELECTE;
	} else {
		hdf_set_value(hdf, PRE_QUERY".uname", uname);
	}
	
	return RET_RBTOP_OK;
}

int member_regist_user(HDF *hdf, mdb_conn *conn)
{
	int ret = member_release_uin(hdf, conn);
	if (ret == RET_RBTOP_OK) {
		hdf_set_copy(hdf, PRE_QUERY".uin", PRE_OUTPUT".uin");
		ret = member_confirm_uin(hdf, conn);
	}
	return ret;
}

int member_alloc_user(HDF *hdf, mdb_conn *conn)
{
	int ret = member_alloc_uin(hdf, conn);
	if (ret == RET_RBTOP_OK) {
		ret = member_confirm_uin(hdf, conn);
		if (ret == RET_RBTOP_OK) {
			hdf_set_copy(hdf, PRE_OUTPUT".uin", PRE_QUERY".uin");
		}
	}
	return ret;
}

int member_check_login(HDF *hdf, mdb_conn *conn)
{
	PRE_DBOP(hdf, conn);

	int uin = hdf_get_int_value(hdf, PRE_QUERY".uin", 0);
	char *usn = hdf_get_value(hdf, PRE_QUERY".usn", "");
	member_t *mb;
	int ret;

	if (uin < MIN_USER_NUM || !strcmp(usn, "")) return RET_RBTOP_INPUTE;

	ret = member_get_info(conn, uin, &mb);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get %d member info failure", uin);
		return RET_RBTOP_SELECTE;
	}

	if (!strcmp(usn, mb->usn)) {
		hdf_set_value(hdf, PRE_QUERY".uname", mb->uname);
		ret = RET_RBTOP_OK;
	} else
		ret = RET_RBTOP_LOGINPSW;

	member_del(mb);
	return ret;
}

void member_after_login(CGI *cgi, mdb_conn *conn)
{
	int uin = hdf_get_int_value(cgi->hdf, PRE_QUERY".uin", 0);
	char *usn = hdf_get_value(cgi->hdf, PRE_QUERY".usn", "");
	if (uin <= MIN_USER_NUM || !strcmp(usn, "")) return;
	
	char mmckey[LEN_MMC_KEY];

	/*
	 * set user's info intoto cookie
	 */
	cgi_cookie_set(cgi, "uin", hdf_get_value(cgi->hdf, PRE_QUERY".uin", "0"),
				   NULL, SITE_DOMAIN, NULL, 1, 0);
	member_set_unameck(cgi);
	member_remember_login(cgi, conn, uin);

	/*
	 * set user's info into mmc
	 * update current hour's count, list
	 */
	char tm[LEN_TM];
	mmisc_getdatetime(tm, sizeof(tm), "%F_%H", 0);
	snprintf(mmckey, sizeof(mmckey), "%s.Count.%s", PRE_MMC_LOGIN, tm);
	mmc_count(MMC_OP_INC, mmckey, 1, NULL, ONE_WEEK, 0);
	
	snprintf(mmckey, sizeof(mmckey), "%s.List.%s", PRE_MMC_LOGIN, tm);
	char val[LEN_INT];
	sprintf(val, "%d ", uin);
	mmc_store(MMC_OP_APP, mmckey, val, 0, ONE_WEEK, 0);
	mtc_warn("store %d's info into mmc failure", uin);

	/*
	 * set user info into hdf
	 */
	hdf_set_copy(cgi->hdf, PRE_OUTPUT".uin", PRE_QUERY".uin");
	hdf_set_copy(cgi->hdf, PRE_OUTPUT".uname", PRE_QUERY".uname");
}

int member_get_info(mdb_conn *conn, int uin, member_t **member)
{
	if (uin < MIN_USER_NUM || member == NULL) {
		mtc_err("input member null");
		return RET_RBTOP_INPUTE;
	}

	char mmckey[LEN_MMC_KEY];
	member_t *mb;
	char *buf;
	int gid;
	NEOERR *err;
	size_t datalen;
	int ret;

	sprintf(mmckey, "%s.%d", PRE_MMC_MEMBER, uin);
	buf = mmc_get(mmckey, &datalen, 0);
	if (buf == NULL || datalen < sizeof(member_t)) {
		if (mdb_get_errcode(conn) != MDB_ERR_NONE) {
			mtc_err("db not init");
			return RET_RBTOP_DBNINIT;
		}
		mb = member_new();
		if (mb == NULL) return RET_RBTOP_MEMALLOCE;
		char usertable[LEN_TB];
		sprintf(usertable, "user_%d", uin%DIV_USER_TB);
		mdb_exec(conn, NULL, "SELECT uin, male, status, uname, usn, musn, "
				 " email, intime, uptime FROM %s WHERE uin=%d;", NULL, usertable, uin);
		ret = mdb_get(conn, "iiiSSSSSS", &mb->uin, &mb->male, &mb->status, &mb->uname,
					  &mb->usn, &mb->musn, &mb->email, &mb->intime, &mb->uptime);
		if (ret != MDB_ERR_NONE) {
			mtc_err("get user %d info failure %s", uin, mdb_get_errmsg(conn));
			free(mb);
			*member = NULL;
			return RET_RBTOP_SELECTE;
		}
		STRING infos;
		string_init(&infos);
		mdb_exec(conn, NULL, "SELECT gid FROM groupinfo WHERE uid=%d;", NULL, uin);
		while (mdb_get_errcode(conn) == MDB_ERR_NONE &&
			   mdb_get(conn, "i", &gid) == MDB_ERR_NONE) {
			string_appendf(&infos, "%d:", gid);
		}
		if (infos.len == 0) string_append(&infos, "none");
		err = string_array_split(&mb->gids, infos.buf, ":", MAX_GROUP_AUSER);
		RETURN_V_NOK(err, RET_RBTOP_GETLISTE);
		member_pack(mb, &buf, &datalen);
		mmc_store(MMC_OP_SET, mmckey, buf, datalen, ONE_DAY, 0);
		string_clear(&infos);
	} else {
		ret = member_unpack(buf, datalen, &mb);
		if (ret != 0) {
			mtc_err("assembly member from mmc error");
			return RET_RBTOP_MMCERR;
		}
	}
	free(buf);
	*member = mb;

	return RET_RBTOP_OK;
}

bool member_has_login(mdb_conn *conn, int uin, char *ckusn)
{
	int ret;
	bool retb;

	if (uin < MIN_USER_NUM || ckusn == NULL || !strcmp(ckusn, ""))
		return false;

	member_t *mb;
	ret = member_get_info(conn, uin, &mb);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get %d member info failure", uin);
		return false;
	}
	
	if (!strcmp(ckusn, mb->musn))
		retb = true;
	else {
		mtc_dbg("cookie's %s not eq sys's %s", ckusn, mb->musn);
		retb = false;
	}

	member_del(mb);
	return retb;
}
bool member_in_group(member_t *mb, int gid)
{
	if (mb == NULL)
		return false;
	
	if (uListIn(mb->gids, (void *)&gid, mmisc_compare_inta) != NULL)
		return true;
	
	return false;
}
bool member_is_owner(member_t *mb, int uid)
{
	if (mb == NULL)
		return false;
	if (mb->uin == uid)
		return true;
	else
		return false;
}
bool member_is_root(int uin)
{
	if (uin == 1001)
		return true;
	else
		return false;
}
