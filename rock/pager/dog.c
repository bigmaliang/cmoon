#include "mheads.h"
#include "lheads.h"

HDF *g_cfg = NULL;

int main(int argc, char *argv[])
{
	int ret;
	
	mconfig_parse_file(SITE_CONFIG, &g_cfg);
	mtc_init(TC_ROOT"dog");

	ret = ltpl_parse_dir(PATH_PAGER, NULL);
	if (ret != RET_RBTOP_OK) {
		mtc_err("parse %s failure %d", PATH_PAGER, ret);
		return 1;
	}

	return 0;
}
