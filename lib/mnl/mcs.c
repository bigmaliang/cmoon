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
void mcs_hdf_escape_val(HDF *hdf)
{
	char *esc = NULL, *val = NULL;
	HDF *child, *next;
	
	if (!hdf) return;

	val = hdf_obj_value(hdf);
	if (val) {
		if (mutil_real_escape_string_nalloc(&esc, val, strlen(val))) {
			hdf_set_value(hdf, NULL, esc);
			free(esc);
		}
	}

	child = hdf_obj_child(hdf);
	if (child) {
		mcs_hdf_escape_val(child);
	}

	next = hdf_obj_next(hdf);
	while (next) {
		mcs_hdf_escape_val(next);
		next = hdf_obj_next(next);
	}
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
