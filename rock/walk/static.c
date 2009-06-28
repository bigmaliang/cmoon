#include "mheads.h"
#include "lheads.h"
#include "ofile.h"

int static_csc_data_get(HDF *hdf, HASH *dbh)
{
	return file_get_nav_by_uri((mdb_conn*)hash_lookup(dbh, "Sys"),
							   "/csc", PRE_OUTPUT, hdf);
}
