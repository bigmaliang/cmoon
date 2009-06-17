#include "mheads.h"
#include "lheads.h"
#include "omember.h"

int member_login_data_get(CGI *cgi, HASH *dbh, session_t *ses)
{
	int ret;
	
	mdb_conn *conn = (mdb_conn*)hash_lookup(dbh, "Sys");
	ret = member_check_login(cgi->hdf, conn);
	if (ret == RET_RBTOP_OK) {
		member_after_login(cgi, conn);
	}
	return ret;
}

int member_logout_data_get(CGI *cgi, HASH *dbh, session_t *ses)
{
	int uin = hdf_get_int_value(cgi->hdf, PRE_COOKIE".uin", 0);
	mtc_info("%d logout", uin);
	member_refresh_info(uin);

	return RET_RBTOP_OK;
}

int member_regist_data_get(CGI *cgi, HASH *dbh, session_t *ses)
{
	int ret;
	
	mdb_conn *conn = (mdb_conn*)hash_lookup(dbh, "Sys");
	ret = member_regist_user(cgi->hdf, conn);
	if (ret == RET_RBTOP_OK) {
		member_after_login(cgi, conn);
	}
	return ret;
}
