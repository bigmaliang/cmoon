#include "mheads.h"
#include "lheads.h"
#include "otjt.h"
#include "ofile.h"

#define TJT_QUERY_COL " id, fid, uid, img, exp, to_char(intime, 'YYYY-MM-DD') " \
    " as intime, to_char(uptime, 'YYYY-MM-DD') as uptime "

#define TJT_GET_RAW(conn, tjt)										\
	mdb_get(conn, "iiiSSSS", &(tjt->id), &(tjt->fid), &(tjt->uid),		\
			&(tjt->img), &(tjt->exp), &(tjt->intime), &(tjt->uptime))

int tjt_get_data(HDF *hdf, HASH *dbh, session_t *ses)
{
    tjt_t *tjt;
    char *buf;
    size_t datalen;
    ULIST *ul = NULL;
	int count, offset, fid, ret;

	mdb_conn *dbsys, *dbtjt;

	dbsys = (mdb_conn*)hash_lookup(dbh, "Sys");
	dbtjt = (mdb_conn*)hash_lookup(dbh, "Tjt");

	PRE_DBOP(hdf, dbsys);
	PRE_DBOP(hdf, dbtjt);

    /* TODO csc to tjt */
	if (ses->file != NULL)
		lutil_fill_layout_by_file(dbsys, ses->file, hdf);
	hdf_set_value(hdf, PRE_OUTPUT".navtitle", "菜色");
	file_get_nav_by_uri(dbsys, "/csc", PRE_OUTPUT, hdf);

	ret = file_check_user_power(hdf, dbsys, ses, ses->file, LMT_APPEND);
	if (ret == RET_RBTOP_OK) {
		hdf_set_value(hdf, PRE_OUTPUT".appendable", "1");
	}

    lutil_fetch_countf(hdf, dbtjt, "tjt", "fid=%d", fid);
	mmisc_get_offset(hdf, &count, &offset);
	fid = ses->file->id;
    
    buf = mmc_getf(&datalen, 0, PRE_MMC_TJT".%d.%d", fid, offset);
    if (buf == NULL || datalen < sizeof(tjt_t)) {
        LDB_QUERY_RAW(dbtjt, "tjt", TJT_QUERY_COL, "fid=%d ORDER BY uptime "
                      " LIMIT %d OFFSET %d", NULL, fid, count, offset);
        mdb_set_rows(hdf, dbtjt, TJT_QUERY_COL, PRE_OUTPUT".items");
        lcs_hdf2list(hdf, PRE_OUTPUT".items", tjt_hdf2item, &ul);
        ret = list_pack(ul, TJT_LEN, tjt_pack_nalloc, &buf, &datalen);
        if (ret == RET_RBTOP_OK) {
            mmc_storef(MMC_OP_SET, buf, datalen, HALF_HOUR, 0, PRE_MMC_TJT".%d.%d",
                       fid, offset);
        }
    } else {
        list_unpack(buf, tjt_unpack, datalen, &ul);
        ret = lcs_list2hdf(ul, PRE_OUTPUT".items", tjt_item2hdf, hdf);
        if (ret != RET_RBTOP_OK) {
            mtc_err("assembly tjt from mmc error");
            return RET_RBTOP_MMCERR;
        }
    }
    uListDestroyFunc(&ul, tjt_del);
    free(buf);
    
    return ret;
}

void tjt_refresh_info(int fid)
{
    /*
     * we refresh first page only here, next pages should wait timeout 
     */
    mmc_deletef(0, PRE_MMC_GROUP".%d.0", fid);
    /* sync with lutil_fetch_countf() */
    mmc_deletef(0, PRE_MMC_COUNT".tjt.fid=%d", fid);
}

int tjt_add_image(CGI *cgi, mdb_conn *conn, session_t *ses)
{
	unsigned char hash[LEN_MD5];
	int ret;

	FILE *fp = cgi_filehandle(cgi, "imagename");
	
	PRE_DBOP(cgi->hdf, conn);
	
	if (fp == NULL) {
		mtc_err("input file named: imagename not found");
		return RET_RBTOP_INPUTE;
	}

	ret = lutil_image_accept(fp, "tjt", hash);
	if (ret != RET_RBTOP_OK) {
		mtc_err("accept image failure %d", ret);
		return ret;
	}
	
	hdf_set_valuef(cgi->hdf, PRE_OUTPUT".imageurl=%s/%s/%s/%s.jpg",
                   IMG_DOMAIN, IMG_PATH, IMG_ORI, hash);
	hdf_set_valuef(cgi->hdf, PRE_OUTPUT".imagename=%s.jpg", hash);
	return RET_RBTOP_OK;
}

int tjt_add_item(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	PRE_DBOP(hdf, conn);

    int fid, uid;
    char *img, *exp;
    int ret;

    uid = ses->member->uin;
    fid = ses->file->id;
    img = hdf_get_value(hdf, PRE_QUERY".img", "");
    exp = hdf_get_value(hdf, PRE_QUERY".exp", "");

    ret = MDATA_SET(conn, EVT_PLUGIN_TJT, NULL, FLAGS_NONE,
                    "INSERT INTO tjt (fid, uid, img, exp) "
                    " VALUES (%d, %d, $1, $2)", "ss",
                    fid, uid, img, exp);
    if (ret != MDB_ERR_NONE) {
        mtc_err("add file err %s", mdb_get_errmsg(conn));
        return RET_RBTOP_INSERTE;
    }
    tjt_refresh_info(fid);
    
	return RET_RBTOP_OK;
}
