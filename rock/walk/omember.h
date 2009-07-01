#ifndef __OMEMBER_H__
#define __OMEMBER_H__
#include "mheads.h"

__BEGIN_DECLS

void member_refresh_info(int uin);
void member_set_unameck(CGI *cgi);
void member_remember_login(CGI *cgi, mdb_conn *conn, int uin);

int  member_release_uin(HDF *hdf, mdb_conn *conn);
int  member_confirm_uin(HDF *hdf, mdb_conn *conn);
int  member_regist_user(HDF *hdf, mdb_conn *conn);
int  member_alloc_user(HDF *hdf, mdb_conn *conn);
int  member_check_login(HDF *hdf, mdb_conn *conn);
void member_after_login(CGI *cgi, mdb_conn *conn);

int  member_get_info(mdb_conn *conn, int uin, member_t **member);

int  member_has_login(HDF *hdf, mdb_conn *conn, session_t *ses);
bool member_is_owner(member_t *mb, int uid);
bool member_in_group_fast(ULIST *gids, ULIST *gmodes, int gid, int mode);
bool member_in_group(member_t *mb, int gid, int mode);
int  member_get_group(member_t *mb, int mode, ULIST **res);
bool member_has_gmode(member_t *mb, int mode);
bool member_uin_is_root(int uin);
bool member_is_root(member_t *mb);

__END_DECLS
#endif	/* __OMEMBER_H__ */
