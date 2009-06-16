#include "mheads.h"
#include "lheads.h"

static int tpl_config(const struct dirent *ent)
{
	if (reg_search(".*.hdf", ent->d_name))
		return 1;
	else
		return 0;
}

int ltpl_parse_dir(char *dir, HASH *outhash)
{
	struct dirent **eps = NULL;
	HDF *node = NULL, *child = NULL, *tmphdf = NULL;
	CSPARSE *cs = NULL;
	char *buf = NULL, *tpl = NULL;
	char fname[_POSIX_PATH_MAX];
	NEOERR *err;
	STRING str;
	int n;

	if (dir == NULL) {
		mtc_err("can't read null directory");
		return RET_RBTOP_INPUTE;
	}

	n = scandir(dir, &eps, tpl_config, alphasort);
	for (int i = 0; i < n; i++) {
		mtc_dbg("rend file %s", eps[i]->d_name);
		cs = NULL; node = NULL;
		memset(fname, 0x0, sizeof(fname));
		snprintf(fname, sizeof(fname), "%s/%s", dir, eps[i]->d_name);

		hdf_init(&node);
		err = hdf_read_file(node, fname);
		JUMP_NOK(err, next);

		child = hdf_obj_child(node);
		while (child != NULL) {
			mtc_dbg("rend node %s", hdf_obj_name(child));
			string_init(&str);
			err = cs_init(&cs, hdf_get_obj(child, PRE_CFG_DATASET));
			JUMP_NOK(err, wnext);
			
			hdf_set_value(cs->hdf, "hdf.loadpaths.local", dir);

			err = cgi_register_strfuncs(cs);
			JUMP_NOK(err, wnext);
			tpl = hdf_get_value(child, PRE_CFG_LAYOUT, "null.html");
			snprintf(fname, sizeof(fname), "%s/%s", dir, tpl);
			err = cs_parse_file(cs, fname);
			JUMP_NOK(err, wnext);

			/*
			 * merge dataset from g_cfg 
			 */
			snprintf(fname, sizeof(fname), PRE_CONFIG"."PRE_CFG_DATASET"_%s", tpl);
			tmphdf = hdf_get_obj(g_cfg, fname);
			if (tmphdf != NULL) hdf_copy(child, PRE_CFG_DATASET, tmphdf);

			/*
			 * do rend 
			 */
			err = cs_render(cs, &str, mcs_strcb);
			JUMP_NOK(err, wnext);

			if (outhash != NULL) {
				buf = calloc(1, str.len);
				if (buf == NULL) {
					mtc_err("oops, memery calloc error");
					goto wnext;
				}
				memcpy(buf, str.buf, str.len);
				/*
				 * strdup the key, baby, because we'll free the hdf later
				 */
				hash_insert(outhash, (void*)strdup(hdf_obj_name(child)), (void*)buf);
			}
			
			if (hdf_get_value(child, PRE_CFG_OUTPUT, NULL) != NULL) {
				snprintf(fname, sizeof(fname), PATH_DOC"%s",
						 hdf_get_value(child, PRE_CFG_OUTPUT, "null.html"));
				mutil_makesure_dir(fname);
				if(!mcs_str2file(str, fname)) {
					mtc_err("write result to %s failure", fname);
				}
			}

		wnext:
			if (cs != NULL) cs_destroy(&cs);
			string_clear(&str);
			child = hdf_obj_next(child);
		}
		
	next:
		if (node != NULL) hdf_destroy(&node);
	}
	if (n > 0) {
		free(eps);
		return RET_RBTOP_OK;
	} else {
		mtc_warn("not .hdf file found in %s", dir);
		return RET_RBTOP_NEXIST;
	}
}

int ltpl_init(HASH **tplh)
{
	HASH *ltplh = NULL;
	NEOERR *err;
	int ret;

	err = hash_init(&ltplh, hash_str_hash, hash_str_comp);
	RETURN_V_NOK(err, RET_RBTOP_INITE);

	ret = ltpl_parse_dir(PATH_TPL, ltplh);
	if (ret != RET_RBTOP_OK) {
		mtc_err("parse %s failure %d", PATH_TPL, ret);
		*tplh = NULL;
		return ret;
	}
	
	*tplh = ltplh;
	return RET_RBTOP_OK;
}

void ltpl_destroy(HASH *tplh)
{
	char *key = NULL;
	
	char *buf = (char*)hash_next(tplh, (void**)&key);

	while (buf != NULL) {
		free(buf);
		buf = hash_next(tplh, (void**)&key);
	}

	hash_destroy(&tplh);
}

int ltpl_render(CGI *cgi, HASH *tplh)
{
	CSPARSE *cs;
	STRING str;
	NEOERR *err;

	char *file, *buf;
	
	/*
	 * how to distinguish /music/zhangwei and /member/zhangwei ?
	 * /music/zhangwei may be use "music"
	 * /member/zhangwei may be use "member"
	 */
	file = hdf_get_value(cgi->hdf, PRE_RSV_RENDER, NULL);
	if (file == NULL) {
		mtc_err("%s not found", PRE_RSV_RENDER);
		return RET_RBTOP_NEXIST;
	}

	buf = (char*)hash_lookup(tplh, file);
	if (buf == NULL) {
		mtc_err("file not found");
		return RET_RBTOP_NEXIST;
	}

	err = cs_init(&cs, cgi->hdf);
	RETURN_V_NOK(err, RET_RBTOP_INITE);

	err = cgi_register_strfuncs(cs);
	/* TODO memory leak */
	RETURN_V_NOK(err, RET_RBTOP_ERROR);

	err = cs_parse_string(cs, buf, strlen(buf));
	RETURN_V_NOK(err, RET_RBTOP_ERROR);

	string_init(&str);
	err = cs_render(cs, &str, mcs_strcb);
	RETURN_V_NOK(err, RET_RBTOP_ERROR);

	err = cgi_output(cgi, &str);
	RETURN_V_NOK(err, RET_RBTOP_ERROR);
	
	cs_destroy(&cs);
	string_clear(&str);

	return RET_RBTOP_OK;
}

