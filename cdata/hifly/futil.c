#include "fheads.h"

int cgi_query_method(CGI *cgi)
{
	char *p = hdf_get_value(cgi->hdf, "CGI.RequestMethod", "NULL");
	if (!strcmp(p, "GET"))
		return QUERY_GET;
	else if (!strcmp(p, "POST"))
		return QUERY_POST;
	else if (!strcmp(p, "PUT"))
		return QUERY_PUT;
	else if (!strcmp(p, "DELETE"))
		return QUERY_DEL;
	else
		return QUERY_UNKNOWN;
}

char* futil_hdf_attr(HDF *hdf, char *name, char*key)
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
char* futil_obj_attr(HDF *hdf, char*key)
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

bool futil_getdatetime(char *res, int len, const char *fmt, time_t second)
{
	memset(res, 0x0, len);
	time_t tm = time(NULL) + second;
	struct tm *stm = localtime(&tm);
	if (strftime(res, len, fmt, stm) == 0)
		return false;
	return true;
}

bool futil_isdigit(char *s)
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

bool futil_is_userdata_key(char *key)
{
	char *p = key+strlen(key)-1;
	while (*p != '_' && p != key) {
		if (!isdigit(*p))
			return false;
		p--;
	}
	return true;
}

void futil_redirect(const char *msg, const char *target, const char *url, bool header)
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

int read_cb(void *ptr, char *data, int size) {return FCGI_fread(data, sizeof(char), size, FCGI_stdin);}
int writef_cb(void *ptr, const char *format, va_list ap) {return FCGI_vprintf(format, ap);}
int write_cb(void *ptr, const char *data, int size) {return FCGI_fwrite((void *)data, sizeof(char), size, FCGI_stdout);}
void futil_wrap_fcgi(int argc, char **argv, char **envp)
{
	cgiwrap_init_std(argc, argv, envp);
	cgiwrap_init_emu(NULL, &read_cb, &writef_cb, &write_cb, NULL, NULL, NULL);
}
