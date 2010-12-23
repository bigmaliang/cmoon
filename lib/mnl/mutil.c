#include "mheads.h"

int CGI_REQ_METHOD(CGI *cgi)
{
	char *opt = hdf_get_value(cgi->hdf, PRE_QUERY".op", "get");
	if (!strcasecmp(opt, "get")) return CGI_REQ_GET;
	else if (!strcasecmp(opt, "add")) return CGI_REQ_PUT;
	else if (!strcasecmp(opt, "del")) return CGI_REQ_DEL;
	else if (!strcasecmp(opt, "mod")) return CGI_REQ_POST;
	
	char *op = hdf_get_value(cgi->hdf, PRE_CGI".RequestMethod", "get");
	if (!strcasecmp(op, "get")) return CGI_REQ_GET;
	else if (!strcasecmp(op, "put")) return CGI_REQ_PUT;
	else if (!strcasecmp(op, "post")) return CGI_REQ_POST;
	else if (!strcasecmp(op, "delete")) return CGI_REQ_DEL;
	
	return CGI_REQ_UNKNOWN;
}

bool mutil_client_attack(HDF *hdf, char *action, char *cname, uint64_t limit, time_t exp)
{
	uint64_t cntcn, cntip; cntcn = cntip = 0;
	char *cn = hdf_get_valuef(hdf, "Cookie.%s", cname);	cn = cn ? cn: "";
	char *ip = hdf_get_value(hdf, "CGI.RemoteAddress", "unknownHost");
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

bool mutil_client_attack_cookie(HDF *hdf, char *action, uint64_t limit, time_t exp)
{
	char scnt[LEN_MMC_KEY], sdur[LEN_MMC_KEY], val[64], tm[LEN_TM_GMT];
	snprintf(scnt, sizeof(scnt), "%s_%s_cnt", PRE_MMC_CLIENT, action);
	snprintf(sdur, sizeof(sdur), "%s_%s_dur", PRE_MMC_CLIENT, action);
	
	char tok[LEN_MMC_KEY];
	snprintf(tok, sizeof(tok), PRE_COOKIE".%s", scnt);
	int cnt = hdf_get_int_value(hdf, tok, 0);
	if (cnt > limit) {
		return true;
	}

	/*
	 * not attack, store increment 
	 */
	if (cnt < 0) cnt = 0;
	sprintf(val, "%d", cnt+1);
	snprintf(tok, sizeof(tok), PRE_COOKIE".%s", sdur);
	char *dur = hdf_get_value(hdf, tok, NULL);
	if (dur == NULL) {
		mmisc_getdatetime_gmt(tm, sizeof(tm), "%A, %d-%b-%Y %T GMT", exp);
		dur = tm;
		cgi_cookie_set(NULL, sdur, dur, NULL, NULL, dur, 1, 0);
	}

	cgi_cookie_set(NULL, scnt, val, NULL, NULL, dur, 1, 0);
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
	strcat(outstr, ";</script>\n");

	printf(outstr);
}

void mutil_md5_str(char *in, char out[LEN_MD5])
{
	if (!in) return;
	
    md5_ctx my_md5;
	unsigned char hexres[16];

	MD5Init(&my_md5);
	MD5Update(&my_md5, (unsigned char*)in, (unsigned int)strlen(in));
	MD5Final(hexres, &my_md5);

	mmisc_hex2str(hexres, 16, (unsigned char*)out);
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
		attr = attr->next;
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
		attr = attr->next;
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

bool mutil_makesure_dir(char *file)
{
	if (file == NULL) return true;

	char tok[_POSIX_PATH_MAX];
	char *p = strchr(file, '/');

	while (p != NULL) {
		memset(tok, 0x0, sizeof(tok));
		strncpy(tok, file, p-file+1);
		if (mkdir(tok, 0755) != 0 && errno != EEXIST) {
			mtc_err("mkdir %s failure %s", tok, strerror(errno));
			return false;
		}
		p = strchr(p+1, '/');
	}
	mtc_noise("directory %s ok", tok);
	return true;
}

void mutil_real_escape_string(char *to, char *from, size_t len)
{
	char escape = 0;
	
	for (size_t i = 0; i < len; i++) {
		escape = 0;
		switch (*(from+i)) {
		case 0:                             /* Must be escaped for 'mysql' */
			escape = '0';
			break;
		case '\n':                          /* Must be escaped for logs */
			escape = 'n';
			break;
		case '\r':
			escape = 'r';
			break;
		case '\\':
			escape = '\\';
			break;
		case '\'':
			escape = '\'';
			break;
		case '"':                           /* Better safe than sorry */
			escape = '"';
			break;
		case '\032':                        /* This gives problems on Win32 */
			escape = 'Z';
			break;
		case ';':
			escape = ';';
			break;
		}
		if (escape) {
			*to++ = '\\';
			*to++= escape;
		} else {
			*to++= *(from+i);
		}
	}
}

char* mutil_real_escape_string_nalloc(char **to, char *from, size_t len)
{
	if (!to || !from) return NULL;

	char *s = calloc(1, len*2+4);
	if (!s) return NULL;

	mutil_real_escape_string(s, from, len);
	*to = s;

	return s;
}

#ifndef DROP_FCGI
int read_cb(void *ptr, char *data, int size) {
	return fread(data, sizeof(char), size, FCGI_stdin);
}
int printf_cb(void *ptr, const char *format, va_list ap) {
	return vprintf(format, ap);
}
int write_cb(void *ptr, const char *data, int size) {
	return fwrite((void *)data, sizeof(char), size, FCGI_stdout);
}
#endif
