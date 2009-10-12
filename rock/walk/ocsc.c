#include "mheads.h"
#include "lheads.h"
#include "ocsc.h"

int csc_add_image(CGI *cgi, mdb_conn *conn, session_t *ses)
{
	unsigned char hash[LEN_MD5];
	int ret;

	FILE *fp = cgi_filehandle(cgi, "imagename");
	
	PRE_DBOP(cgi->hdf, conn);
	
	if (fp == NULL) {
		mtc_err("input file named: imagename not found");
		return RET_RBTOP_INPUTE;
	}

	ret = lutil_image_accept(fp, "csc", hash);
	if (ret != RET_RBTOP_OK) {
		mtc_err("accept image failure %d", ret);
		return ret;
	}
	
	hdf_set_valuef(cgi->hdf, PRE_OUTPUT".imageurl=%s/%s/%s/%s.jpg",
                   IMG_DOMAIN, IMG_PATH, IMG_ORI, hash);
	hdf_set_valuef(cgi->hdf, PRE_OUTPUT".imagename=%s/%s/%s.jpg",
                   IMG_PATH, IMG_ORI, hash);
	return RET_RBTOP_OK;
}

int csc_add_item(CGI *cgi, mdb_conn *conn, session_t *ses)
{
	PRE_DBOP(cgi->hdf, conn);

    int fid, uid;
    char *img, *exp;
    int ret;

    uid = ses->member->uin;
    fid = ses->file->id;
    img = hdf_get_value(cgi->hdf, PRE_QUERY".img", "");
    exp = hdf_get_value(cgi->hdf, PRE_QUERY".exp", "");

    ret = MDATA_SET(conn, EVT_PLUGIN_CSC, NULL, FLAGS_NONE,
                    "INSERT INTO tjt (fid, uid, img, exp) "
                    " VALUES (%d, %d, $1, $2)", "ss",
                    fid, uid, img, exp);
    if (ret != MDB_ERR_NONE) {
        mtc_err("add file err %s", mdb_get_errmsg(conn));
        return RET_RBTOP_INSERTE;
    }
    
	return RET_RBTOP_OK;
}
