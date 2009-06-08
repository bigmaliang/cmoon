#include "mheads.h"
#include "lheads.h"

#include "ofile.h"

int CGI_REQ_TYPE(CGI *cgi)
{
	char *tp = hdf_get_value(cgi->hdf, PRE_REQ_TYPE, "html");
	if (!strcmp(tp, "html"))
		return CGI_REQ_HTML;
	else if (!strcmp(tp, "ajax"))
		return CGI_REQ_AJAX;
	else
		return CGI_REQ_UNSUPPORT;
}

int lutil_file_check_power(CGI *cgi, mdb_conn *conn, char *uri, bool split)
{
	ULIST *urls, *files;
	file_t *file;
	int listlen;
	char *url;

	char errmsg[LEN_MD];
	NEOERR *err;
	int i, ret, reqmethod;

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
		ret = file_get_info_uri(conn, uri, &file);
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
	hdf_set_value(cgi->hdf, PRE_REQ_FILE, file->name);
	
	reqmethod = CGI_REQ_METHOD(cgi);
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
	return RET_RBTOP_OK;
	
notpass:
	hdf_remove_tree(cgi->hdf, PRE_SUCCESS);
	if (urls != NULL)
		uListDestroy(&urls, ULIST_FREE);
	if (files != NULL)
		uListDestroyFunc(&files, file_del);
	return ret;
}

int lutil_file_access(CGI *cgi, mdb_conn *conn)
{
	char *uri = hdf_get_value(cgi->hdf, PRE_REQ_URI, NULL);
	if (uri == NULL || strlen(uri) < strlen(CGI_RUN_DIR)+1) {
		mtc_err("uri %s illegal", uri);
		return RET_RBTOP_INITE;
	}

	/* ignore /run in uri */
	return lutil_file_check_power(cgi, conn, uri+strlen(CGI_RUN_DIR)+1, true);
}

int lutil_file_access_rewrited(CGI *cgi, HASH *dbh)
{
	mdb_conn *conn = (mdb_conn*)hash_lookup(dbh, "Sys");
	if (conn == NULL) {
		mtc_err("sys db can't found");
		return RET_RBTOP_INITE;
	}

	char *uri = hdf_get_value(cgi->hdf, PRE_REQ_URI_RW, NULL);
	return lutil_file_check_power(cgi, conn, uri, false);
}

void* lutil_get_data_handler(void *lib, CGI *cgi)
{
	char *file, *tp;
	char hname[_POSIX_PATH_MAX];
	void *res;

	/* TODO how to division /music/zhangwei and /member/zhangwei ? */
	file = hdf_get_value(cgi->hdf, PRE_REQ_FILE, NULL);
	if (file == NULL) {
		mtc_err("%s not found", PRE_REQ_FILE);
		return NULL;
	}
	
	switch (CGI_REQ_METHOD(cgi)) {
		case CGI_REQ_GET:
			snprintf(hname, sizeof(hname), "%s_data_get", file);
			break;
		case CGI_REQ_POST:
			snprintf(hname, sizeof(hname), "%s_data_mod", file);
			break;
		case CGI_REQ_PUT:
			snprintf(hname, sizeof(hname), "%s_data_add", file);
			break;
		case CGI_REQ_DEL:
			snprintf(hname, sizeof(hname), "%s_data_del", file);
			break;
		default:
			mtc_err("op not support");
			return NULL;
	}

	res = dlsym(lib, hname);
	if ((tp = dlerror()) != NULL) {
		mtc_err("%s not found %s", hname, tp);
		return NULL;
	}
	return res;
}

int lutil_render(CGI *cgi, HASH *tplh)
{
	CSPARSE *cs;
	STRING str;
	NEOERR *err;

	char *file, *buf;
	
	/* TODO how to division /music/zhangwei and /member/zhangwei ? */
	file = hdf_get_value(cgi->hdf, PRE_REQ_FILE, NULL);
	if (file == NULL) {
		mtc_err("%s not found", PRE_REQ_FILE);
		return RET_RBTOP_NEXIST;
	}

	buf = (char*)hash_lookup(tplh, file);
	if (buf == NULL) {
		mtc_err("file not found");
		return RET_RBTOP_NEXIST;
	}

	err = cs_init(&cs, cgi->hdf);
	RETURN_V_NOK(err, RET_RBTOP_INITE);

	err = cgi_register_strfuncs(cs);
	/* TODO memory leak */
	RETURN_V_NOK(err, RET_RBTOP_ERROR);

	err = cs_parse_string(cs, buf, strlen(buf));
	RETURN_V_NOK(err, RET_RBTOP_ERROR);

	string_init(&str);
	err = cs_render(cs, &str, mcs_strcb);
	RETURN_V_NOK(err, RET_RBTOP_ERROR);

	err = cgi_output(cgi, &str);
	RETURN_V_NOK(err, RET_RBTOP_ERROR);
	
	cs_destroy(&cs);
	string_clear(&str);

	return RET_RBTOP_OK;
}

int lutil_init_db(HASH **dbh)
{
	HDF *node;
	HASH *ldbh;
	NEOERR *err;
	int ret;

	node = hdf_get_obj(g_cfg, "Db");
	if (node == NULL) {
		mtc_err("Db config not found");
		return RET_RBTOP_INITE;
	}
	
	err = hash_init(&ldbh, hash_str_hash, hash_str_comp);
	RETURN_V_NOK(err, RET_RBTOP_INITE);

	node = hdf_obj_child(node);
	mdb_conn *conn;
	bool filled = false;
	while (node != NULL) {
		ret = mdb_init(&conn, hdf_obj_value(node));
		if (ret == RET_RBTOP_OK) {
			hash_insert(ldbh, (void*)strdup(hdf_obj_name(node)), (void*)conn);
			filled = true;
		}
		
		node = hdf_obj_next(node);
	}

	if (!filled) {
		mtc_err("no valid db connection");
		return RET_RBTOP_INITE;
	}
	*dbh = ldbh;
	return RET_RBTOP_OK;
}

void lutil_cleanup_db(HASH *dbh)
{
	char *key = NULL;
	
	mdb_conn *conn = (mdb_conn*)hash_next(dbh, (void**)&key);

	while (conn != NULL) {
		mdb_destroy(conn);
		conn = hash_next(dbh, (void**)&key);
	}

	hash_destroy(&dbh);
}

static int tpl_config(const struct dirent *ent)
{
	if (reg_search(".*.hdf", ent->d_name))
		return 1;
	else
		return 0;
}

int lutil_init_tpl(HASH **tplh)
{
	struct dirent **eps = NULL;
	HDF *node = NULL, *child = NULL;
	CSPARSE *cs = NULL;
	HASH *ltplh = NULL;
	char *buf = NULL;
	char fname[_POSIX_PATH_MAX];
	STRING str;
	NEOERR *err;
	int n;

	err = hash_init(&ltplh, hash_str_hash, hash_str_comp);
	RETURN_V_NOK(err, RET_RBTOP_INITE);

	n = scandir(PATH_TPL, &eps, tpl_config, alphasort);
	for (int i = 0; i < n; i++) {
		cs = NULL; node = NULL;
		memset(fname, 0x0, sizeof(fname));
		snprintf(fname, sizeof(fname), "%s/%s", PATH_TPL, eps[i]->d_name);

		hdf_init(&node);
		err = hdf_read_file(node, fname);
		JUMP_NOK(err, next);

		child = hdf_obj_child(node);

		while (child != NULL) {
			string_init(&str);
			err = cs_init(&cs, child);
			JUMP_NOK(err, wnext);

			err = cgi_register_strfuncs(cs);
			JUMP_NOK(err, wnext);
			err = cs_parse_file(cs, F_TPL_LAYOUT);
			JUMP_NOK(err, wnext);

			err = cs_render(cs, &str, mcs_strcb);
			JUMP_NOK(err, wnext);

			buf = calloc(1, str.len);
			if (buf == NULL) {
				mtc_err("oops, memery calloc error");
				goto wnext;
			}
			memcpy(buf, str.buf, str.len);
			/*
			 * strdup the key, baby, because we'll free the hdf later
			 */
			hash_insert(ltplh, (void*)strdup(hdf_obj_name(child)), (void*)buf);

		wnext:
			if (cs != NULL) cs_destroy(&cs);
			string_clear(&str);
			child = hdf_obj_next(child);
		}
		
	next:
		if (node != NULL) hdf_destroy(&node);
	}
	if (n > 0) free(eps);
	else mtc_warn("not .hdf file found in %s", PATH_TPL);
	
	*tplh = ltplh;
	return RET_RBTOP_OK;
}

void lutil_cleanup_tpl(HASH *tplh)
{
	char *key = NULL;
	
	char *buf = (char*)hash_next(tplh, (void**)&key);

	while (buf != NULL) {
		free(buf);
		buf = hash_next(tplh, (void**)&key);
	}

	hash_destroy(&tplh);
}

