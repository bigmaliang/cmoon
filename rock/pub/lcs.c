#include "mheads.h"
#include "lheads.h"

int lcs_set_layout_infoa(HDF *hdf, const char *title, anchor_t *crumbs,
						 anchor_t *g_nav, int navnum)
{
	int ret;
	NEOERR *err;
	int i;

	hdf_set_value(hdf, "hdf.loadpaths.0", PATH_FRT_TPL);
	hdf_set_value(hdf, PRE_LAYOUT".title", title);
	//ret = mcs_set_login_info(hdf);

	char key[LEN_ST];
	HDF *node;
	for (i = 0; i < navnum; i++) {
		sprintf(key, "%s.tabs.%d", PRE_LAYOUT, i);
		err = hdf_get_node(hdf, key, &node);
		if (err != STATUS_OK) {
			mtc_err("create %dst node for layout.tabs failure");
			continue;
		}
		if (!strcmp(g_nav[i].name, crumbs[0].name)) {
			hdf_set_value(node, "class", "selected");
		}
		hdf_set_value(node, "href", g_nav[i].href);
		hdf_set_value(node, "name", g_nav[i].name);
	}

	i = -1;
	while (strcmp(crumbs[++i].name, "")) {
		sprintf(key, "%s.crumbs.%d", PRE_LAYOUT, i);
		err = hdf_get_node(hdf, key, &node);
		if (err != STATUS_OK) {
			mtc_err("create %dst node for layout.crumbs failure");
			continue;
		}
		hdf_set_value(node, "href", crumbs[i].href);
		hdf_set_value(node, "name", crumbs[i].name);
	}
	
	return ret;
}
