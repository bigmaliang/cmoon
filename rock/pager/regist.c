#include "mheads.h"
#include "lheads.h"

anchor_t g_crumbs[3] = {
	{"市中心", "/index.html", "", ""},
	{"上户口", "/regist.html", "", ""},
	{"", "", "", ""}
};

int main(int argc, char *argv[])
{
	HDF *hdf;
	CSPARSE *cs;
	NEOERR *err;

	mtc_init(TC_ROOT"pager/regist");
	mcfg_init(SITE_CONFIG);

	err = hdf_init(&hdf);
	if (err != STATUS_OK) {
		nerr_log_error(err);
		return -1;
	}

	lcs_set_layout_infoa(hdf, "Baby, join me in death", g_crumbs, g_nav, NAV_NUM);
	hdf_set_value(hdf, PRE_INCLUDE".css", PATH_TPL"regist.css.html");
	hdf_set_value(hdf, PRE_INCLUDE".js", PATH_TPL"regist.js.html");
	hdf_set_value(hdf, PRE_INCLUDE".content", PATH_TPL"regist.html");

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

	if(!mcs_str2file(str, PATH_DOC"regist.html")) {
		mtc_err("write result to out file failure");
	}

	cs_destroy(&cs);
	hdf_destroy(&hdf);
	string_clear(&str);
	return 0;
}
