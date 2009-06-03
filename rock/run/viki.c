/*
 * viki 主脑
 */

#include "mheads.h"
#include "lheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv, char **envp)
{
	CGI *cgi;
	NEOERR *err;
	HASH *dbh;
	char *file;
	int ret;

	int (*req_handler)(CGI *cgi, HASH *dbh);

	mtc_init(TC_ROOT"viki");
	mutil_wrap_fcgi(argc, argv, envp);
	mconfig_parse_file(SITE_CONFIG, &g_cfg);

	ret = lutil_init_db(&dbh);
	if (ret != RET_RBTOP_OK) {
		mtc_err("init db error");
		mutil_redirect("初始化数据库失败", TGT_SELF, URL_CLOSE, true);
		return ret;
	}

#ifndef NFCGI
	while (FCGI_Accept() >= 0) {
#endif
		err = cgi_init(&cgi, NULL);
		JUMP_NOK_CGI(err, opfinish);
		err = cgi_parse(cgi);
		JUMP_NOK_CGI(err, opfinish);

		ret = lutil_file_access_rewrited(cgi, dbh);
		file = hdf_get_value(cgi->hdf, PRE_REQFILE, NULL);
		if (ret != RET_RBTOP_OK || file == NULL) {
			goto opfinish;
		}

		switch (CGI_REQ_METHOD(cgi)) {
		case CGI_REQ_GET:
			break;
		case CGI_REQ_POST:
			break;
		case CGI_REQ_PUT:
			break;
		case CGI_REQ_DEL:
			break;
		default:
			ret = RET_RBTOP_INPUTE;
			break;
		}
		
	opfinish:
		if (cgi != NULL) {
			char *cb = hdf_get_value(cgi->hdf, PRE_QUERY".jsoncallback", NULL);
			if (cb != NULL) {
				mjson_execute_hdf(cgi->hdf, cb);
			} else {
				mjson_output_hdf(cgi->hdf);
			}
			cgi_destroy(&cgi);
		}
#ifndef NFCGI
	}
#endif

	lutil_cleanup_db(dbh);
	mconfig_cleanup(&g_cfg);
	return 0;
}


