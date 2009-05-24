#include "mheads.h"

int CGI_REQ_METHOD(CGI *cgi)
{
	char *op = hdf_get_value(cgi->hdf, PRE_CGI".RequestMethod", REQ_GET);
	if (REQ_IS_GET(op)) {
		return CGI_REQ_GET;
	} else if (REQ_IS_PUT(op)) {
		return CGI_REQ_PUT;
	} else if (REQ_IS_POST(op)) {
		return CGI_REQ_POST;
	} else if (REQ_IS_DEL(op)) {
		return CGI_REQ_DEL;
	}
	return CGI_REQ_UNKNOWN;
}

bool mutil_client_attack(HDF *hdf, char *action, int limit, time_t exp)
{
	char key[LEN_MMC_KEY];
	char *cn = hdf_get_value(hdf, "Cookie.ClientName", "");
	char *ip = hdf_get_value(hdf, "CGI.RemoteAddress", "unknown host");
	int cntcn, cntip; cntcn = cntip = 0;
	snprintf(key, sizeof(key), "%s.%s.%s", PRE_MMC_CLIENT, action, cn);
	mmc_count(MMC_OP_INC, key, 1, (uint64_t*)&cntcn, exp, 0);
	snprintf(key, sizeof(key), "%s.%s.%s", PRE_MMC_CLIENT, action, ip);
	mmc_count(MMC_OP_INC, key, 1, (uint64_t*)&cntip, exp, 0);
	if (cntcn >= limit || cntip >= limit) {
		hdf_set_int_value(hdf, PRE_OUTPUT".tired", cntcn);
		hdf_set_int_value(hdf, PRE_OUTPUT".limit", limit);
		hdf_set_int_value(hdf, PRE_OUTPUT".during", exp/60);
		return true;
	}
	return false;
}

void mutil_redirect(const char *msg, const char *target, const char *url, bool header)
{
	char outstr[LEN_MD];
	char tok[LEN_SM];
	
	if (header) {
		printf("Content-Type: text/html; charset=UTF-8\n\n");
	}
	strcpy(outstr, "<script language='javascript'>");
	
	if (msg != NULL) {
		snprintf(tok, sizeof(tok), "alert('%s');", msg);
		strcat(outstr, tok);
	}
	
	strcat(outstr, "window.");
	
	if (target != NULL) {
		strncat(outstr, target, sizeof(tok));
	} else {
		strcat(outstr, TGT_SELF);
	}
	
	if (!strcmp(url, URL_BLANK) ||
		!strcmp(url, URL_RELOAD) ||
		!strcmp(url, URL_CLOSE) ||
		!strcmp(url, URL_BACK)) {
		strncat(outstr, url, sizeof(tok));
	} else {
		snprintf(tok, sizeof(tok), "location.href='%s'", url);
		strcat(outstr, tok);
	}
	strcat(outstr, ";</script>");

	printf(outstr);
}
