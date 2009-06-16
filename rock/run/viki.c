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
	int ret;

	HASH *dbh;
	HASH *tplh;
	char *requri, *jsoncb;

	int (*data_handler)(CGI *cgi, HASH *dbh);
	void *lib;

	mutil_wrap_fcgi(argc, argv, envp);

	mconfig_parse_file(SITE_CONFIG, &g_cfg);
	mtc_init(TC_ROOT"viki");

	ret = ltpl_init(&tplh);
	if (ret != RET_RBTOP_OK) {
		mtc_err("init templates error");
		mutil_redirect("初始化模板失败", TGT_SELF, URL_CLOSE, true);
		return ret;
	}

	ret = ldb_init(&dbh);
	if (ret != RET_RBTOP_OK) {
		mtc_err("init db error");
		mutil_redirect("初始化数据库失败", TGT_SELF, URL_CLOSE, true);
		return ret;
	}

	lib = dlopen(NULL, RTLD_NOW|RTLD_GLOBAL);
	if (lib == NULL) {
		mtc_err("possible? %s", dlerror());
		mutil_redirect("初始化库函数失败", TGT_SELF, URL_CLOSE, true);
		return 1;
	}
	
#ifndef NFCGI
	while (FCGI_Accept() >= 0) {
#endif
		err = cgi_init(&cgi, NULL);
		JUMP_NOK_CGI(err, response);
		err = cgi_parse(cgi);
		JUMP_NOK_CGI(err, response);

		requri = hdf_get_value(cgi->hdf, PRE_REQ_URI_RW, "NULL");
		if (mutil_client_attack(cgi->hdf, requri, LMT_CLI_ATTACK,
								PERIOD_CLI_ATTACK)) {
			goto response;
		}
		
		ret = lfile_access_rewrited(cgi, dbh);
		if (ret != RET_RBTOP_OK) {
			goto response;
		}

		data_handler = lutil_get_data_handler(lib, cgi);
		if (data_handler == NULL) {
			mtc_err("get handler failure");
			ret = RET_RBTOP_NEXIST;
			goto response;
		}

		ret = (*data_handler)(cgi, dbh);
		
	response:
		if (cgi != NULL && cgi->hdf != NULL) {
			switch (CGI_REQ_TYPE(cgi)) {
			case CGI_REQ_HTML:
				if (ret != RET_RBTOP_OK && ret == RET_RBTOP_NEXIST) {
					cgi_redirect(cgi, "/404.html");
				} else {
					ret = ltpl_render(cgi, tplh);
					if (ret != RET_RBTOP_OK) {
						if (ret == RET_RBTOP_NEXIST)
							cgi_redirect(cgi, "/404.html");
						else
							cgi_redirect(cgi, "/503.html");
					}
				}
				break;
			case CGI_REQ_AJAX:
				ldb_opfinish_json(ret, cgi->hdf, NULL);
				jsoncb = hdf_get_value(cgi->hdf, PRE_REQ_AJAX_FN, NULL);
				if (jsoncb != NULL) {
					mjson_execute_hdf(cgi->hdf, jsoncb);
				} else {
					mjson_output_hdf(cgi->hdf);
				}
				break;
			default:
				cgi_redirect(cgi, "/503.html");
				break;
			}
			cgi_destroy(&cgi);
		}
#ifndef NFCGI
	}
#endif

	ldb_destroy(dbh);
	ltpl_destroy(tplh);
	mconfig_cleanup(&g_cfg);
	return 0;
}


