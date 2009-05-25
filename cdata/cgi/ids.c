#include "mheads.h"
#include "lheads.h"
#include "oids.h"

int main()
{
	CGI *cgi = NULL;
	NEOERR *err;
	fdb_t *fdb = NULL;
	int ret;

	//sleep(20);
	mtc_init(HF_LOG_PATH"ids");
	
	cgi_init(&cgi, NULL);
	err = cgi_parse(cgi);
	if (err != STATUS_OK) {
		mtc_err("init cgi error");
		cgi_neo_error(cgi, err);
		return 1;
	}

	ret = ids_fdb_init(&fdb);
	fdb_opfinish_json(ret, cgi->hdf, fdb);

	ret = ids_get_data(cgi->hdf, fdb);
	fdb_opfinish_json(ret, cgi->hdf, fdb);

	mjson_output_hdf(cgi->hdf);

	cgi_destroy(&cgi);
	fdb_free(&fdb);
	return 0;
}
