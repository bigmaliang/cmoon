#include "mheads.h"
#include "lheads.h"
#include "comment.h"

HDF *g_cfg;

int main()
{
	CGI *cgi;
	NEOERR *err;
	int ret;

	//sleep(20);
	mtc_init(TC_ROOT"comment");
	mcfg_init(SITE_CONFIG);
	atexit(exiting);

	err = cgi_init(&cgi, NULL);
	DIE_NOK_CGI(err);
	err = cgi_parse(cgi);
	DIE_NOK_CGI(err);

	mdb_conn *conn = NULL;
	ret = mdb_init(&conn, DB_DSN);
	mdb_opfinish_json(ret, cgi->hdf, conn);

	//hdf_set_value(cgi->hdf, PRE_QUERY".op", "show");
	char *op = hdf_get_value(cgi->hdf, PRE_QUERY".op", "");
	if (!strcmp(op, "show")) {
		ret = comment_get(cgi->hdf, conn);
	} else if (!strcmp(op, "add")) {
		ret = comment_add(cgi->hdf, conn);
	} else {
		ret = RET_RBTOP_INPUTE;
	}
	mdb_opfinish_json(ret, cgi->hdf, conn);
	
	//mdb_destroy(conn);
	mjson_output_hdf(cgi->hdf, 0);

#ifdef DEBUG_HDF
	hdf_write_file(cgi->hdf, TC_ROOT"hdf.comment");
#endif
	
	cgi_destroy(&cgi);
	return 0;
}
