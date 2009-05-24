#include "mheads.h"
#include "lheads.h"
#include "member.h"

int main(int argc, char *argv[])
{
	CGI *cgi;
	NEOERR *err;
	int ret;

	//sleep(20);
	mtc_init(TC_ROOT"userlogin");
	mcfg_init(SITE_CONFIG);

	err = cgi_init(&cgi, NULL);
	DIE_NOK_CGI(err);
	err = cgi_parse(cgi);
	DIE_NOK_CGI(err);

	if (mutil_client_attack(cgi->hdf, "userlogin", LMT_CLI_LOGIN, ONE_HOUR)) {
		mtc_warn("%s %s seems to be a blood attack",
				 hdf_get_value(cgi->hdf, "CGI.RemoteAddress", ""),
				 hdf_get_value(cgi->hdf, "Cookie.ClientName", ""));
	} else {
		/* process regist apply */
		mdb_conn *conn = NULL;
		ret = mdb_init(&conn, DB_DSN_SYS);
		ldb_opfinish_json(ret, cgi->hdf, conn);
		
		char *op = hdf_get_value(cgi->hdf, PRE_QUERY".op", "");
		if (!strcmp(op, "login")) {
			ret = member_check_login(cgi->hdf, conn);
			if (ret == RET_RBTOP_OK) {
				member_after_login(cgi, conn);
			}
		} else if (!strcmp(op, "forget")) {
#if 0
			ret = member_forget(cgi->hdf, conn);
			if (ret == RET_RBTOP_OK) {
				member_after_forget(cgi);
			}
#endif
		} else {
			ret = RET_RBTOP_INPUTE;
		}
		ldb_opfinish_json(ret, cgi->hdf, conn);
		
		mdb_destroy(conn);
	}
	mjson_output_hdf(cgi->hdf);

#ifdef DEBUG_HDF
	hdf_write_file(cgi->hdf, TC_ROOT"regist.hdf");
#endif

	cgi_destroy(&cgi);
	return 0;
}
