#include "mheads.h"
#include "lheads.h"
#include "ocsc.h"

int csc_add_image(CGI *cgi, mdb_conn *conn, session_t *ses)
{
	unsigned char hash[16];
	int ret;

	FILE *fp = cgi_filehandle(cgi, "imagename");
	
	PRE_DBOP(cgi->hdf, conn);
	
	if (fp == NULL) {
		mtc_err("input file named: imagename not found");
		return RET_RBTOP_INPUTE;
	}
	
	ret = lutil_image_accept(fp, "/data/pic/csc/", hash);
	if (ret != RET_RBTOP_OK) {
		mtc_err("accept image failure %d", ret);
		return ret;
	}
	
	hdf_set_valuef(cgi->hdf, PRE_OUTPUT".imageurl=/pic/%s.jpg", hash);
	return RET_RBTOP_OK;
}

int csc_add_item(CGI *cgi, mdb_conn *conn, session_t *ses)
{
	PRE_DBOP(cgi->hdf, conn);
	
	return RET_RBTOP_OK;
}
