#include "mheads.h"
#include "lheads.h"
#include "oids.h"

HDF *g_cfg = NULL;

int main()
{
	CGI *cgi = NULL;
	NEOERR *err;
	ids_db_t *dbt = NULL;
	int ret;

	//sleep(20);
	mtc_init(HF_LOG_PATH"ids");
	if (!mconfig_parse_file(CONFIG_FILE, &g_cfg)) {
		mtc_err("init config %s error", CONFIG_FILE);
		printf("Content-Type: text/html; charset=UTF-8\r\n\r\n");
		printf("{errmsg: \"初始化配置失败\"}");
		return 1;
	}
	
	cgi_init(&cgi, NULL);
	err = cgi_parse(cgi);
	if (err != STATUS_OK) {
		mtc_err("init cgi error");
		cgi_neo_error(cgi, err);
		return 1;
	}

	ret = ids_dbt_init(&dbt);
	fdb_opfinish_json(ret, cgi->hdf, NULL);

	ret = ids_get_data(cgi->hdf, dbt);
	fdb_opfinish_json(ret, cgi->hdf, NULL);

	mjson_output_hdf(cgi->hdf);

	cgi_destroy(&cgi);
	dbt_free(dbt);
	mconfig_cleanup(&g_cfg);
	return 0;
}
