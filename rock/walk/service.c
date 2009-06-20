#include "mheads.h"
#include "lheads.h"
#include "ofile.h"

int service_action_data_get(CGI *cgi, HASH *dbh, session_t *ses)
{
	return file_get_action(cgi->hdf,
						   (mdb_conn*)hash_lookup(dbh, "Sys"), ses);
}
