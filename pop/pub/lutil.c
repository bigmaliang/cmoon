#include "mheads.h"
#include "lheads.h"

/*
 * TODO how make local dlsym ok? so tired 
 */
static void lutil_donotcall()
{
	app_exist_data_get(NULL, NULL, NULL, NULL);
}

int CGI_REQ_TYPE(CGI *cgi, HDF *rcfg)
{
	int ret;
	
	if (cgi == NULL) return CGI_REQ_UNSUPPORT;
	
	ret = hdf_get_int_value(cgi->hdf, PRE_RSV_REQ_TYPE, CGI_REQ_UNSUPPORT);
	if (ret == CGI_REQ_UNSUPPORT) {
		char file[_POSIX_PATH_MAX];
		snprintf(file, sizeof(file), "%s.reqtype",
				 hdf_get_value(cgi->hdf, PRE_REQ_URI_RW_HDF, "UNKNOWN"));
		ret = hdf_get_int_value(rcfg, file, 0);
	}
	return ret;
}

void* lutil_get_data_handler(void *lib, CGI *cgi, HDF *rcfg)
{
	char *file, *dataer = NULL, *tp;
	char hname[_POSIX_PATH_MAX];
	void *res;

	file = hdf_get_value(cgi->hdf, PRE_REQ_URI_RW_HDF, NULL);
	if (file) {
		dataer = hdf_get_valuef(rcfg, "%s.dataer", file);
	}
	if (!dataer) {
		mtc_err("%s dataer not found", file);
		return NULL;
	}
	
	switch (CGI_REQ_METHOD(cgi)) {
		case CGI_REQ_GET:
			snprintf(hname, sizeof(hname), "%s_data_get", dataer);
			break;
		case CGI_REQ_POST:
			snprintf(hname, sizeof(hname), "%s_data_mod", dataer);
			break;
		case CGI_REQ_PUT:
			snprintf(hname, sizeof(hname), "%s_data_add", dataer);
			break;
		case CGI_REQ_DEL:
			snprintf(hname, sizeof(hname), "%s_data_del", dataer);
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
