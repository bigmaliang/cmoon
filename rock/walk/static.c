#include "mheads.h"
#include "lheads.h"
#include "ofile.h"

int static_csc_data_get(HDF *hdf, HASH *dbh)
{
	hdf_set_value(hdf, PRE_QUERY".uri", "/csc");
	return file_get_nav_by_uri(hdf, (mdb_conn*)hash_lookup(dbh, "Sys"));
}
