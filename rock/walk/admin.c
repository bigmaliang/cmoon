#include "mheads.h"
#include "lheads.h"
#include "omember.h"
#include "ofile.h"
#include "ogroup.h"

int admin_account_data_add(CGI *cgi, HASH *dbh, session_t *ses)
{
	return member_alloc_user(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"));
}


int admin_file_data_get(CGI *cgi, HASH *dbh, session_t *ses)
{
	int ret;
	ret = file_get_files(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"), ses);
	file_translate_mode(cgi->hdf);
	return ret;
}

int admin_file_data_mod(CGI *cgi, HASH *dbh, session_t *ses)
{
	return file_modify(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"), ses);
}

int admin_file_data_add(CGI *cgi, HASH *dbh, session_t *ses)
{
	return file_add(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"), ses);
}

int admin_file_data_del(CGI *cgi, HASH *dbh, session_t *ses)
{
	return file_delete(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"), ses);
}



int admin_group_data_get(CGI *cgi, HASH *dbh, session_t *ses)
{
	return group_get_groups(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"), ses);
}

int admin_group_data_mod(CGI *cgi, HASH *dbh, session_t *ses)
{
	return group_add_member(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"), ses);
}

int admin_group_data_add(CGI *cgi, HASH *dbh, session_t *ses)
{
	return group_add_member(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"), ses);
}

int admin_group_data_del(CGI *cgi, HASH *dbh, session_t *ses)
{
	return group_del_member(cgi->hdf, (mdb_conn*)hash_lookup(dbh, "Sys"), ses);
}
