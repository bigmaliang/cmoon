#include "mheads.h"
#include "lheads.h"
#include "olimit.h"

int main(int argc, char *argv[])
{
	CGI *cgi;
	NEOERR *err;
	int ret;
	mdb_conn *conn = NULL;
	char *opt;

	//sleep(20);
	mtc_init(TC_ROOT"sys.limit");
	mcfg_init(SITE_CONFIG);

	err = cgi_init(&cgi, NULL);
	DIE_NOK_CGI(err);
	err = cgi_parse(cgi);
	DIE_NOK_CGI(err);

#ifdef NCGI_MODE
	hdf_set_value(cgi->hdf, PRE_CGI".RequestMethod", REQ_GET);
	hdf_set_value(cgi->hdf, PRE_CGI".ScriptName", "/run/sys/limit");
	hdf_set_value(cgi->hdf, PRE_COOKIE".uin", "1001");
	hdf_set_value(cgi->hdf, PRE_COOKIE".uname", "bigml");
	char *musn_esc;
	cgi_url_escape("`^lg47 3pMZkN]( 6 }D|ms", &musn_esc);
	hdf_set_value(cgi->hdf, PRE_COOKIE".musn", musn_esc);
	free(musn_esc);
#endif

	ret = mdb_init(&conn, DB_DSN_SYS);
	ldb_opfinish_json(ret, cgi->hdf, conn);
	
	lutil_file_access_json(cgi, conn);

	switch(CGI_REQ_METHOD(cgi)) {
	case CGI_REQ_GET:
		ret = limit_get_limits(cgi->hdf, conn);
		limit_translate_mode(cgi->hdf);
		break;
	case CGI_REQ_POST:
		/* TODO boa not support put, del method... */
		opt = hdf_get_value(cgi->hdf, PRE_QUERY".op", "unknown");
		if (!strcmp(opt, "add")) {
			ret = limit_add(cgi->hdf, conn);
		} else if (!strcmp(opt, "del")) {
			ret = limit_delete(cgi->hdf, conn);
		} else {
			ret = limit_modify(cgi->hdf, conn);
		}
		break;
	default:
		ret = RET_RBTOP_INPUTE;
		break;
	}
	ldb_opfinish_json(ret, cgi->hdf, conn);

	mdb_destroy(conn);
	mjson_output_hdf(cgi->hdf);
	cgi_destroy(&cgi);
	return 0;
}
