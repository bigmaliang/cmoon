#include "mheads.h"
#include "lheads.h"

anchor_t g_crumbs[2] = {
	{"申请流程", "/process.html", "", ""},
	{"", "", "", ""}
};

HDF *g_cfg;

int main(int argc, char *argv[])
{
	HDF *hdf;
	CSPARSE *cs;
	NEOERR *err;

	mtc_init(TC_ROOT"pager_process");

	err = hdf_init(&hdf);
	if (err != STATUS_OK) {
		nerr_log_error(err);
		return -1;
	}

	lcs_set_layout_infoa(hdf, "申请流程", g_crumbs, g_nav, NAV_NUM);
	hdf_set_value(hdf, PRE_INCLUDE".content", PATH_FRT_TPL"process.html");

	err = cs_init(&cs, hdf);
	DIE_NOK_MTL(err);
	err = cgi_register_strfuncs(cs);
	DIE_NOK_MTL(err);

	err = cs_parse_file(cs, F_TPL_LAYOUT);
	DIE_NOK_MTL(err);

	STRING str;
	string_init(&str);
	err = cs_render(cs, &str, mcs_strcb);
	DIE_NOK_MTL(err);

	if(!mcs_str2file(str, PATH_FRT_DOC"process.html")) {
		mtc_err("write result to out file failure");
	}

	cs_destroy(&cs);
	hdf_destroy(&hdf);
	string_clear(&str);
	return 0;
}
