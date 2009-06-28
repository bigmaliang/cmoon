#include "mheads.h"
#include "lheads.h"
#include "ofile.h"

//#include "ocsc.h"

int csc_data_get(CGI *cgi, HASH *dbh, session_t *ses)
{
	if (ses->file != NULL)
		lutil_fill_layout_by_file((mdb_conn*)hash_lookup(dbh, "Sys"),
								  ses->file, cgi->hdf);
	hdf_set_value(cgi->hdf, PRE_OUTPUT".navtitle", "菜色");
	return file_get_nav_by_uri((mdb_conn*)hash_lookup(dbh, "Sys"),
						"/csc", PRE_OUTPUT, cgi->hdf);
	//return csc_get_data(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Csc"));

}
