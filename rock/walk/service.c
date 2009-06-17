#include "mheads.h"
#include "lheads.h"

int service_action_data_get(CGI *cgi, HASH *dbh, session_t *ses)
{
	int uin = hdf_get_int_value(cgi->hdf, PRE_QUERY".uin", 0);
	if (uin == 1001 || uin == 9999) {
		hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.name", "号码管理");
		hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.href", "/admin/account.html");
		hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.1.name", "文件管理");
		hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.1.href", "/admin/file.html");
	} else if (uin%100 == 0) {
		hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.name", "管理分类信息");
		hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.href", "/run/sortmanage");
	} else {
		hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.name", "管理内容");
		hdf_set_value(cgi->hdf, PRE_OUTPUT".actions.0.href", "/run/mnsmanage");
	}
	return RET_RBTOP_OK;
}
