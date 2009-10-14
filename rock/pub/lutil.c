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

/* make sure result has enough capacity */
int lutil_image_accept(FILE *fp, char *path, unsigned char *result)
{
	if (fp == NULL || result == NULL || path == NULL)
		return RET_RBTOP_INPUTE;

    md5_ctx my_md5;
	unsigned char data[4096], hexres[16];
	unsigned int bytes;

	memset(data, 0x0, sizeof(data));
    MD5Init(&my_md5);

	fseek(fp, 0, SEEK_SET);
	while ((bytes = fread(data, 1, 4096, fp)) != 0) {
		MD5Update(&my_md5, data, bytes);
		memset(data, 0x0, sizeof(data));
	}
	memset(hexres, 0x0, 16);
    MD5Final(hexres, &my_md5);

	mmisc_hex2str(hexres, 16, result);

	char fname[LEN_FN];
	snprintf(fname, sizeof(fname), IMG_ROOT"%s/%s/%s.jpg", path, IMG_ORI, result);
	FILE *fpout = fopen(fname, "w+");
	if (fpout == NULL) {
		mtc_err("open %s for write failure", fname);
		return RET_RBTOP_CREATEFE;
	}

	fseek(fp, 0, SEEK_SET);
	while ((bytes = fread(data, 1, 4096, fp)) != 0)
		fwrite(data, 1, bytes, fpout);
	fclose(fpout);

    /*
     * take sooo long time, so, process it backend later async
     */
#if 0
    char tok[LEN_FN];
    int ret;
    snprintf(tok, sizeof(tok), PATH_MTLS"imagemagick.sh %s %s.jpg >> /tmp/imagemagick.log 2>&1",
             path, result);
    ret = system(tok);
    if (ret != 0) {
        mtc_err("process %s failure %d", result, ret);
        return RET_RBTOP_IMGPROE;
    }
#endif

	return RET_RBTOP_OK;
}

int lutil_fetch_count(HDF *hdf, mdb_conn *conn, char *table, char *cond)
{
	PRE_DBOP(hdf, conn);
    if (table == NULL || cond == NULL)
        return RET_RBTOP_INPUTE;

    char *buf;
    size_t datalen;
    int count = 0;
    
    buf = mmc_getf(&datalen, 0, PRE_MMC_COUNT".%s.%s", table, cond);
    if (buf == NULL) {
        mdb_exec(conn, NULL, "SELECT count(*) FROM %s WHERE %s;",
                 NULL, table, cond);
        mdb_get(conn, "s", &buf);
        count = atoi(buf);
        mmc_storef(MMC_OP_SET, (void*)buf, 0, ONE_HOUR, 0,
                   PRE_MMC_COUNT".%s.%s", table, cond);
    } else {
        count = atoi(buf);
    }

    hdf_set_int_value(hdf, PRE_OUTPUT".ttnum", count);

    return RET_RBTOP_OK;
}

int lutil_fetch_countf(HDF *hdf, mdb_conn *conn, char *table, char *cfmt, ...)
{
    char cond[LEN_SM];
    va_list ap;

    va_start(ap, cfmt);
    vsnprintf(cond, sizeof(cond), cfmt, ap);
    va_end(ap);

    return lutil_fetch_count(hdf, conn, table, cond);
}
