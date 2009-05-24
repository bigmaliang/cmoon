#include "mheads.h"
#include "lheads.h"
#include "member.h"

anchor_t g_crumbs[3] = {
	{"市中心", "/index.html", "", ""},
	{"管理用户", "/run/useradmin", "", ""},
	{"", "", "", ""}
};

int main(int argc, char *argv[])
{
	CGI *cgi;
	NEOERR *err;
	int ret;

	//sleep(20);
	mtc_init(TC_ROOT"useradmin");
	mcfg_init(SITE_CONFIG);

	err = cgi_init(&cgi, NULL);
	DIE_NOK_CGI(err);
	err = cgi_parse(cgi);
	DIE_NOK_CGI(err);

	/* process manage apply */
	mdb_conn *conn = NULL;
	ret = mdb_init(&conn, DB_DSN_SYS);
	ldb_opfinish_json(ret, cgi->hdf, conn);

	bool json = true;
	char *op = hdf_get_value(cgi->hdf, PRE_QUERY".op", "");
	if (!strcmp(op, "alloc")) {
		ret = member_alloc_user(cgi->hdf, conn);
	} else {
		json = false;
		lcs_set_layout_infoa(cgi->hdf, "管理用户号码", g_crumbs, g_nav, NAV_NUM);
		hdf_set_value(cgi->hdf, PRE_LAYOUT".js", PATH_TPL"alloc.js");
		hdf_set_value(cgi->hdf, PRE_INCLUDE".content", PATH_TPL"alloc.html");

		err = cgi_display(cgi, F_TPL_LAYOUT);
		DIE_NOK_CGI(err);
	}
	if (json) {
		ldb_opfinish_json(ret, cgi->hdf, conn);
		mjson_output_hdf(cgi->hdf);
	}
		
	mdb_destroy(conn);

	cgi_destroy(&cgi);
	return 0;
}
