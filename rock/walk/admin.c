#include "mheads.h"
#include "lheads.h"
#include "omember.h"
#include "ofile.h"

int admin_account_data_add(CGI *cgi, HASH *dbh)
{
	return member_alloc_user(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"));
}


int admin_file_data_get(CGI *cgi, HASH *dbh)
{
	int ret;
	ret = file_get_files(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"));
	file_translate_mode(cgi->hdf);
	return ret;
}

int admin_file_data_mod(CGI *cgi, HASH *dbh)
{
	return file_modify(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"));
}

int admin_file_data_add(CGI *cgi, HASH *dbh)
{
	return file_add(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"));
}

int admin_file_data_del(CGI *cgi, HASH *dbh)
{
	return file_delete(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"));
}

