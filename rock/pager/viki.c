#include "mheads.h"
#include "lheads.h"

HDF *g_cfg = NULL;

int main(int argc, char *argv[])
{
	struct dirent **eps = NULL;
	HDF *node = NULL, *child = NULL, *tmphdf = NULL;
	CSPARSE *cs = NULL;
	char fname[_POSIX_PATH_MAX];
	char *tpl = NULL;
	STRING str;
	NEOERR *err;
	int n;

	mconfig_parse_file(SITE_CONFIG, &g_cfg);
	mtc_init(TC_ROOT"pager.viki");

	n = scandir("./", &eps, tpl_config, alphasort);
	for (int i = 0; i < n; i++) {
		mtc_dbg("rend file %s", eps[i]->d_name);
		cs = NULL; node = NULL;
		memset(fname, 0x0, sizeof(fname));
		
		hdf_init(&node);
		err = hdf_read_file(node, eps[i]->d_name);
		JUMP_NOK(err, next);

		child = hdf_obj_child(node);
		while (child != NULL) {
			mtc_info("rend node %s", hdf_obj_name(child));
			string_init(&str);
			err = cs_init(&cs, hdf_get_obj(child, PRE_CFG_DATASET));
			JUMP_NOK(err, wnext);

			err = cgi_register_strfuncs(cs);
			JUMP_NOK(err, wnext);
			tpl = hdf_get_value(child, PRE_CFG_LAYOUT, "null.html");
			err = cs_parse_file(cs, tpl);
			JUMP_NOK(err, wnext);

			/*
			 * merge dataset from g_cfg 
			 */
			snprintf(fname, sizeof(fname), PRE_CONFIG"."PRE_CFG_DATASET"_%s", tpl);
			tmphdf = hdf_get_obj(g_cfg, fname);
			if (tmphdf != NULL) hdf_copy(child, PRE_CFG_DATASET, tmphdf);

			err = cs_render(cs, &str, mcs_strcb);
			JUMP_NOK(err, wnext);

			snprintf(fname, sizeof(fname), PATH_DOC"%s",
					 hdf_get_value(child, PRE_CFG_OUTPUT, "null.html"));
			mutil_makesure_dir(fname);
			if(!mcs_str2file(str, fname)) {
				mtc_err("write result to %s failure", fname);
			}

		wnext:
			if (cs != NULL) cs_destroy(&cs);
			string_clear(&str);
			child = hdf_obj_next(child);
		}
		
	next:
		if (node != NULL) hdf_destroy(&node);
	}
	
	return 0;
}
