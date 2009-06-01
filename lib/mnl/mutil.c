#include "mheads.h"

int CGI_REQ_METHOD(CGI *cgi)
{
	char *op = hdf_get_value(cgi->hdf, PRE_CGI".RequestMethod", REQ_GET);
	/* TODO boa not support put, del method... */
	char *opt = hdf_get_value(cgi->hdf, PRE_QUERY".op", REQ_GET);
	if (REQ_IS_GET(op)) {
		return CGI_REQ_GET;
	} else if (REQ_IS_PUT(op) || !strcmp(opt, "add")) {
		return CGI_REQ_PUT;
	} else if (REQ_IS_POST(op)) {
		return CGI_REQ_POST;
	} else if (REQ_IS_DEL(op) || !strcmp(opt, "del")) {
		return CGI_REQ_DEL;
	}
	return CGI_REQ_UNKNOWN;
}

bool mutil_client_attack(HDF *hdf, char *action, uint64_t limit, time_t exp)
{
	uint64_t cntcn, cntip; cntcn = cntip = 0;
	char *cn = hdf_get_value(hdf, "Cookie.ClientName", "");
	char *ip = hdf_get_value(hdf, "CGI.RemoteAddress", "unknown host");
	mmc_countf(MMC_OP_INC, 1, &cntcn, exp, 0, "%s.%s.%s", PRE_MMC_CLIENT, action, cn);
	mmc_countf(MMC_OP_INC, 1, &cntip, exp, 0, "%s.%s.%s", PRE_MMC_CLIENT, action, ip);
	if (cntcn >= limit || cntip >= limit) {
		hdf_set_int_value(hdf, PRE_OUTPUT".tired", cntcn);
		hdf_set_int_value(hdf, PRE_OUTPUT".limit", limit);
		hdf_set_int_value(hdf, PRE_OUTPUT".during", exp);
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

char* mutil_hdf_attr(HDF *hdf, char *name, char*key)
{
	if (hdf == NULL || name == NULL || key == NULL)
		return NULL;
	
	HDF_ATTR *attr = hdf_get_attr(hdf, name);
	while (attr != NULL) {
		if (!strcmp(attr->key, key)) {
			return attr->value;
		}
	}
	return NULL;
}
char* mutil_obj_attr(HDF *hdf, char*key)
{
	if (hdf == NULL || key == NULL)
		return NULL;
	
	HDF_ATTR *attr = hdf_obj_attr(hdf);
	while (attr != NULL) {
		if (!strcmp(attr->key, key)) {
			return attr->value;
		}
	}
	return NULL;
}

bool mutil_isdigit(char *s)
{
	if (s == NULL)
		return false;
	
	char *p = s;
	while (*p != '\0') {
		if (!isdigit((int)*p))
			return false;
		p++;
	}
	return true;
}

int read_cb(void *ptr, char *data, int size) {return FCGI_fread(data, sizeof(char), size, FCGI_stdin);}
int writef_cb(void *ptr, const char *format, va_list ap) {return FCGI_vprintf(format, ap);}
int write_cb(void *ptr, const char *data, int size) {return FCGI_fwrite((void *)data, sizeof(char), size, FCGI_stdout);}
void mutil_wrap_fcgi(int argc, char **argv, char **envp)
{
	cgiwrap_init_std(argc, argv, envp);
	cgiwrap_init_emu(NULL, &read_cb, &writef_cb, &write_cb, NULL, NULL, NULL);
}
