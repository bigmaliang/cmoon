#include "mheads.h"
#include "lheads.h"

#include "ofile.h"

#include "honghong.h"
/*
 * TODO how make local dlsym ok? so tired 
 */
static void lutil_donotcall()
{
	admin_account_data_add(NULL, NULL, NULL);
}

int CGI_REQ_TYPE(CGI *cgi)
{
	if (cgi == NULL) return CGI_REQ_UNSUPPORT;
	return hdf_get_int_value(cgi->hdf, PRE_RSV_REQ_TYPE, 0);
}

void* lutil_get_data_handler(void *lib, CGI *cgi)
{
	char *file, *tp;
	char hname[_POSIX_PATH_MAX];
	void *res;

	/*
	 * how to distinguish /music/zhangwei and /member/zhangwei ?
	 * /music/zhangwei may be use music_data_xxx
	 * /member/zhangwei may be use member_data_xxx
	 */
	file = hdf_get_value(cgi->hdf, PRE_RSV_DATAER, NULL);
	if (file == NULL) {
		mtc_err("%s not found", PRE_RSV_DATAER);
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
		mtc_err("%s", tp);
		return NULL;
	} else
		mtc_info("%s found for data handler", hname);
	return res;
}

int lutil_fill_layout_by_file(mdb_conn *conn, file_t *file, HDF *hdf)
{
	ULIST *files = NULL;
	file_t *fl;
	int ret, errsn;

	PRE_DBOP(hdf, conn);
	if (file == NULL)
		return RET_RBTOP_INPUTE;

	hdf_set_value(hdf, PRE_LAYOUT".title", file->remark);

	ret = file_get_infos_by_uri(conn, file->uri, &files, &errsn);
	if (ret != RET_RBTOP_OK) {
		mtc_err("get files's infos by uri %s failure %d",
				file->uri, errsn);
		return ret;
	}

	MLIST_ITERATE(files, fl) {
		hdf_set_valuef(hdf, "%s.crumbs.%d.name=%s",
					   PRE_LAYOUT, t_rsv_i, fl->remark);
		if (fl->reqtype == CGI_REQ_AJAX) {
			hdf_set_valuef(hdf, "%s.crumbs.%d.href=%s.html",
						   PRE_LAYOUT, t_rsv_i, fl->uri);
		} else {
			hdf_set_valuef(hdf, "%s.crumbs.%d.href=%s",
						   PRE_LAYOUT, t_rsv_i, fl->uri);
		}
	}

	if (files != NULL)
		uListDestroy(&files, ULIST_FREE);
	return RET_RBTOP_OK;
}

