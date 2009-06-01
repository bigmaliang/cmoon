#include "mheads.h"
#include "lheads.h"

#include "ofile.h"

void lutil_file_access_json(CGI *cgi, mdb_conn *conn)
{
	ULIST *urls, *files;
	file_t *file;
	int listlen;
	char *url;

	char errmsg[LEN_MD];
	NEOERR *err;
	int i, ret, reqmethod;

	urls = files = NULL;

	char *uri = hdf_get_value(cgi->hdf, PRE_CGI".ScriptName", NULL);
	if (uri == NULL) {
		mtc_err("input uri null");
		goto notpass;
	}
	err = string_array_split(&urls, uri, URI_SPLITER, MAX_URI_ITEM);
	RETURN_NOK(err);

	listlen = uListLength(urls);
	err = uListGet(urls, 0, (void**)&url);
	RETURN_NOK(err);
	if (listlen < 1 || strcmp(url, CGI_RUN_DIR)) {
		mtc_warn("%s not a valid request", uri);
		hdf_set_value(cgi->hdf, PRE_ERRMSG, "非法请求");
		goto notpass;
	}

	/* get splited url's file info */
	ret = file_get_infos(conn, urls, &files, &i);
	if (ret != RET_RBTOP_OK) {
		uListGet(urls, i, (void**)&url);
		mtc_info("get file for %s failure", url);
		snprintf(errmsg, sizeof(errmsg)-1, "对不起, %s 权限验证失败", url);
		hdf_set_value(cgi->hdf, PRE_ERRMSG, errmsg);
		goto notpass;
	}

	reqmethod = CGI_REQ_METHOD(cgi);
	uListGet(files, uListLength(files)-1, (void**)&file);
	//RETURN_NOK(err);
	if (reqmethod == CGI_REQ_GET) {
		ret = file_check_user_power(cgi, conn, file, LMT_GET);
		if(ret != RET_RBTOP_OK) {
			if (ret == RET_RBTOP_NOTLOGIN)
				strcpy(errmsg, "敏感操作, 请先登录");
			else
				snprintf(errmsg, sizeof(errmsg)-1, "对不起, 您无权读取 %s", file->remark);
			hdf_set_value(cgi->hdf, PRE_ERRMSG, errmsg);
			goto notpass;
		}
	} else if (reqmethod == CGI_REQ_POST) {
		ret = file_check_user_power(cgi, conn, file, LMT_MOD);
		if(ret != RET_RBTOP_OK) {
			if (ret == RET_RBTOP_NOTLOGIN)
				strcpy(errmsg, "敏感操作, 请先登录");
			else
				snprintf(errmsg, sizeof(errmsg)-1, "对不起, 您无权修改 %s", file->remark);
			hdf_set_value(cgi->hdf, PRE_ERRMSG, errmsg);
			goto notpass;
		}
	} else if (reqmethod == CGI_REQ_PUT) {
		ret = file_check_user_power(cgi, conn, file, LMT_APPEND);
		if(ret != RET_RBTOP_OK) {
			if (ret == RET_RBTOP_NOTLOGIN)
				strcpy(errmsg, "敏感操作, 请先登录");
			else
				snprintf(errmsg, sizeof(errmsg)-1, "对不起, 您无权增加 %s", file->remark);
			hdf_set_value(cgi->hdf, PRE_ERRMSG, errmsg);
			goto notpass;
		}
	} else if (reqmethod == CGI_REQ_DEL) {
		ret = file_check_user_power(cgi, conn, file, LMT_DEL);
		if(ret != RET_RBTOP_OK) {
			if (ret == RET_RBTOP_NOTLOGIN)
				strcpy(errmsg, "敏感操作, 请先登录");
			else
				snprintf(errmsg, sizeof(errmsg)-1, "对不起, 您无权删除 %s", file->remark);
					
			hdf_set_value(cgi->hdf, PRE_ERRMSG, errmsg);
			goto notpass;
		}
	} else {
		hdf_set_value(cgi->hdf, PRE_ERRMSG, "请求类型非法");
		goto notpass;
	}

	if (urls != NULL)
		uListDestroy(&urls, ULIST_FREE);
	if (files != NULL)
		uListDestroyFunc(&files, file_del);
	return;
	
notpass:
	hdf_remove_tree(cgi->hdf, PRE_SUCCESS);
	mjson_output_hdf(cgi->hdf);
	if (urls != NULL)
		uListDestroy(&urls, ULIST_FREE);
	if (files != NULL)
		uListDestroyFunc(&files, file_del);
	/* TODO system resource need free*/
	exit(1);
}
