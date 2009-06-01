#include "mheads.h"
#include "lheads.h"
#include "olimit.h"

HDF *g_cfg = NULL;

int main(int argc, char *argv[])
{
	CGI *cgi;
	NEOERR *err;
	int ret;
	mdb_conn *conn = NULL;

	//sleep(20);
	mtc_init(TC_ROOT"sys.limit");
	mconfig_parse_file(SITE_CONFIG, &g_cfg);

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

	ret = mdb_init(&conn, DB_SYS);
	ldb_opfinish_json(ret, cgi->hdf, conn);
	
	lutil_file_access_json(cgi, conn);

	switch(CGI_REQ_METHOD(cgi)) {
	case CGI_REQ_GET:
		ret = limit_get_limits(cgi->hdf, conn);
		limit_translate_mode(cgi->hdf);
		break;
	case CGI_REQ_POST:
		ret = limit_modify(cgi->hdf, conn);
		break;
	case CGI_REQ_PUT:
		ret = limit_add(cgi->hdf, conn);
		break;
	case CGI_REQ_DEL:
		ret = limit_delete(cgi->hdf, conn);
		break;
	default:
		ret = RET_RBTOP_INPUTE;
		break;
	}
	ldb_opfinish_json(ret, cgi->hdf, conn);

	mdb_destroy(conn);
	mjson_output_hdf(cgi->hdf);
	cgi_destroy(&cgi);
	mconfig_cleanup(&g_cfg);
	return 0;
}
