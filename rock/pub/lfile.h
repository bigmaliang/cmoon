#ifndef __LFILE_H__
#define __LFILE_H__
#include "mheads.h"

__BEGIN_DECLS

int lfile_check_power(CGI *cgi, mdb_conn *conn, session_t *ses, char *uri);
int lfile_access(CGI *cgi, mdb_conn *conn, session_t *ses);
int lfile_access_rewrited(CGI *cgi, HASH *dbh, session_t *ses);
/*
 * should be call after ses->member, ses->file inited
 * hdf: Uiplug.x.  {uri = xxx  mole = {0...} atom = {0...}}
 */
int lfile_filter_uiplug(HDF *hdf, session_t *ses);

__END_DECLS
#endif	/* __LFILE_H__ */
