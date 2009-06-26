#include "mheads.h"
#include "lheads.h"
#include "ofile.h"

int lfile_check_power(CGI *cgi, mdb_conn *conn, session_t *ses, char *uri, bool split)
{
	ULIST *urls, *files;
	file_t *file;
	int listlen;
	char *url;

	char errmsg[LEN_MD];
	NEOERR *err;
	int i, ret = RET_RBTOP_OK, reqmethod;

	urls = files = NULL;

	if (uri == NULL) {
		mtc_err("input uri null");
		return RET_RBTOP_INPUTE;
	}
	if (split) {
		err = string_array_split(&urls, uri, URI_SPLITER, MAX_URI_ITEM);
		RETURN_V_NOK(err, RET_RBTOP_INPUTE);
		listlen = uListLength(urls);
		if (listlen < 1) {
			mtc_warn("%s not a valid request", uri);
			hdf_set_value(cgi->hdf, PRE_ERRMSG, "非法请求");
			goto notpass;
		}

		/* get splited url's file info */
		ret = file_get_infos(conn, urls, &files, &i);
		if (ret != RET_RBTOP_OK) {
			uListGet(urls, i, (void**)&url);
			mtc_warn("get file for %s failure", url);
			if (ret == RET_RBTOP_NEXIST)
				snprintf(errmsg, sizeof(errmsg)-1, " %s 不存在", url);
			else
				snprintf(errmsg, sizeof(errmsg)-1, "对不起, %s 权限验证失败", url);
			hdf_set_value(cgi->hdf, PRE_ERRMSG, errmsg);
			goto notpass;
		}
		uListGet(files, uListLength(files)-1, (void**)&file);
	} else {
		ret = file_get_info_by_uri(conn, uri, &file);
		if (ret != RET_RBTOP_OK) {
			mtc_warn("get file for %s failure", uri);
			if (ret == RET_RBTOP_NEXIST)
				hdf_set_valuef(cgi->hdf, PRE_ERRMSG"= %s 不存在", uri);
			else
				hdf_set_valuef(cgi->hdf, PRE_ERRMSG"=对不起, %s 权限验证失败", uri);
			goto notpass;
		}
	}

	/*
	 * set file name for later use 
	 */
	hdf_set_value(cgi->hdf, PRE_RSV_FILE, file->name);
	hdf_set_int_value(cgi->hdf, PRE_RSV_REQ_TYPE, file->reqtype);
	hdf_set_value(cgi->hdf, PRE_RSV_DATAER, file->dataer);
	hdf_set_value(cgi->hdf, PRE_RSV_RENDER, file->render);
	
	reqmethod = CGI_REQ_METHOD(cgi);
	if (reqmethod == CGI_REQ_GET) {
		ret = file_check_user_power(cgi->hdf, conn, ses, file, LMT_GET);
		if(ret != RET_RBTOP_OK) {
			if (ret == RET_RBTOP_NOTLOGIN)
				strcpy(errmsg, "敏感操作, 请先登录");
			else
				snprintf(errmsg, sizeof(errmsg)-1, "对不起, 您无权读取 %s",
						 file->remark);
			hdf_set_value(cgi->hdf, PRE_ERRMSG, errmsg);
			goto notpass;
		}
	} else if (reqmethod == CGI_REQ_POST) {
		ret = file_check_user_power(cgi->hdf, conn, ses, file, LMT_MOD);
		if(ret != RET_RBTOP_OK) {
			if (ret == RET_RBTOP_NOTLOGIN)
				strcpy(errmsg, "敏感操作, 请先登录");
			else
				snprintf(errmsg, sizeof(errmsg)-1, "对不起, 您无权修改 %s",
						 file->remark);
			hdf_set_value(cgi->hdf, PRE_ERRMSG, errmsg);
			goto notpass;
		}
	} else if (reqmethod == CGI_REQ_PUT) {
		ret = file_check_user_power(cgi->hdf, conn, ses, file, LMT_APPEND);
		if(ret != RET_RBTOP_OK) {
			if (ret == RET_RBTOP_NOTLOGIN)
				strcpy(errmsg, "敏感操作, 请先登录");
			else
				snprintf(errmsg, sizeof(errmsg)-1, "对不起, 您无权增加 %s",
						 file->remark);
			hdf_set_value(cgi->hdf, PRE_ERRMSG, errmsg);
			goto notpass;
		}
	} else if (reqmethod == CGI_REQ_DEL) {
		ret = file_check_user_power(cgi->hdf, conn, ses, file, LMT_DEL);
		if(ret != RET_RBTOP_OK) {
			if (ret == RET_RBTOP_NOTLOGIN)
				strcpy(errmsg, "敏感操作, 请先登录");
			else
				snprintf(errmsg, sizeof(errmsg)-1, "对不起, 您无权删除 %s",
						 file->remark);
					
			hdf_set_value(cgi->hdf, PRE_ERRMSG, errmsg);
			goto notpass;
		}
	} else {
		hdf_set_value(cgi->hdf, PRE_ERRMSG, "请求类型非法");
		goto notpass;
	}

	if (!split) {
		ses->file = file;
		HDF *node = hdf_get_obj(g_cfg, PRE_CFG_FILECACHE);
		if (node != NULL) node = hdf_obj_child(node);
		while (node != NULL) {
			if (reg_search(hdf_get_value(node, "uri", "NULL"), file->uri)) {
				ses->tm_cache_browser = hdf_get_int_value(node, "tm_cache", 0);
				break;
			}
			node = hdf_obj_next(node);
		}
	}
	if (urls != NULL)
		uListDestroy(&urls, ULIST_FREE);
	if (files != NULL)
		uListDestroyFunc(&files, file_del);
	return RET_RBTOP_OK;
	
notpass:
	hdf_remove_tree(cgi->hdf, PRE_SUCCESS);
	if (urls != NULL)
		uListDestroy(&urls, ULIST_FREE);
	if (files != NULL)
		uListDestroyFunc(&files, file_del);
	return ret;
}

int lfile_access(CGI *cgi, mdb_conn *conn, session_t *ses)
{
	char *uri = hdf_get_value(cgi->hdf, PRE_REQ_URI, NULL);
	if (uri == NULL || strlen(uri) < strlen(CGI_RUN_DIR)+1) {
		mtc_err("uri %s illegal", uri);
		return RET_RBTOP_INITE;
	}

	/* ignore /run in uri */
	return lfile_check_power(cgi, conn, ses, uri+strlen(CGI_RUN_DIR)+1, true);
}

int lfile_access_rewrited(CGI *cgi, HASH *dbh, session_t *ses)
{
	mdb_conn *conn = (mdb_conn*)hash_lookup(dbh, "Sys");
	if (conn == NULL) {
		mtc_err("sys db can't found");
		return RET_RBTOP_INITE;
	}

	char *uri = hdf_get_value(cgi->hdf, PRE_REQ_URI_RW, NULL);
	return lfile_check_power(cgi, conn, ses, uri, false);
}

