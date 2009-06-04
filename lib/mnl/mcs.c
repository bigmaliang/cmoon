#include "mheads.h"

NEOERR* mcs_outputcb(void *ctx, char *s)
{
	printf ("%s", s);
	return STATUS_OK;
}
NEOERR* mcs_strcb(void *ctx, char *s)
{
	STRING *str = (STRING*)ctx;
	NEOERR *err;
	err = nerr_pass(string_append(str, s));
	return err;
}
bool mcs_str2file(STRING str, const char *file)
{
	if (file == NULL)
		return false;
	
	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
		mtc_err("unable to open %s for write", file);
		return false;
	}
	size_t ret = fwrite(str.buf, str.len, 1, fp);
	if (ret < 0) {
		mtc_err("write str.buf to %s error", file);
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}

int mcs_set_login_info(HDF *hdf)
{
	int uin = hdf_get_int_value(hdf, "Cookie.uin", 0);
	char *uname = hdf_get_value(hdf, "Cookie.uname", "");
	if (uin != 0 && strcmp(uname, "")) {
		hdf_set_value(hdf, PRE_LAYOUT".member.uname", uname);
		return RET_USER_LOGIN;
	}
	return RET_USER_NLOGIN;
}
