#include "mheads.h"
#include "lheads.h"

int main(int argc, char *argv[])
{
	CGI *cgi;
	NEOERR *err;
	int ret;

	//sleep(20);
	mtc_init(TC_ROOT"usermenu");
	mcfg_init(SITE_CONFIG);

	err = cgi_init(&cgi, NULL);
	DIE_NOK_CGI(err);
	err = cgi_parse(cgi);
	DIE_NOK_CGI(err);

	char *op = hdf_get_value(cgi->hdf, PRE_CGI".RequestMethod", REQ_GET);
	int uin = hdf_get_int_value(cgi->hdf, PRE_QUERY".uin", 0);
	ret = RET_RBTOP_OK;
	if (REQ_IS_GET(op)) {
		hdf_set_value(cgi->hdf, PRE_SUCCESS, "1");
		if (uin == 1001 || uin == 9999) {
			hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.name", "管理开拓者");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.href", "/run/pionnermanage");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.1.name", "号码管理");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.1.href", "/run/useradmin");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.2.name", "权限管理");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.2.href", "/sys/limit.html");
		} else if (uin%100 == 0) {
			hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.name", "管理分类信息");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.href", "/run/sortmanage");
		} else {
			hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.name", "管理内容");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.href", "/run/mnsmanage");
		}
	} else {
		hdf_set_value(cgi->hdf, PRE_ERRMSG, "输入参数错误");
	}
	mjson_output_hdf(cgi->hdf);
	
	cgi_destroy(&cgi);
	return 0;
}
