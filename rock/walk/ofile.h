#ifndef __OFILE_H__
#define __OFILE_H__
#include "mheads.h"

__BEGIN_DECLS

#define LMT_MASK	0xFF
#define LMT_GET		0x1
#define LMT_MOD		0x2
#define LMT_APPEND	0x4
#define LMT_DEL		0x8

#define PMS_ALL(mode)		(mode & LMT_MASK)
#define PMS_MEMBER(mode)	((mode>>8) & LMT_MASK)
#define PMS_JOIN(mode)		((mode>>16) & LMT_MASK)

#define MODE_ALL(mode)		(mode & LMT_MASK)
#define MODE_MEMBER(mode)	((mode & LMT_MASK) << 8)
#define MODE_JOIN(mode)		((mode & LMT_MASK) << 16)
/*
 * user have priviliage >= GROUP_MODE_SENIOR can do ANY Not-system operation
 */

int file_check_user_power(HDF *hdf, mdb_conn *conn, session_t *ses,
						  file_t *file, int access);

/*
 * low level file operater, by id/ pid+name/ uri
 */
int  file_get_info_by_id(mdb_conn *conn, int id, char *url, int pid, file_t **file);
int  file_get_info_by_uri(mdb_conn *conn, char *uri, file_t **file);
int  file_get_infos_by_list(mdb_conn *conn, ULIST *urls, ULIST **files, int *noksn);
int  file_get_infos_by_uri(mdb_conn *conn, char *uri, ULIST **files, int *noksn);
void file_refresh_me(file_t *fl);
void file_refresh_info(mdb_conn *conn, int id, char *url, int pid);

/*
 * manage user's file: add, mod, del 
 */
int  file_get_files(HDF *hdf, mdb_conn *conn, session_t *ses);
void file_translate_mode(HDF *hdf);
int  file_modify(HDF *hdf, mdb_conn *conn, session_t *ses);
int  file_add(HDF *hdf, mdb_conn *conn, session_t *ses);
int  file_delete(HDF *hdf, mdb_conn *conn, session_t *ses);

/*
 * get logined user's action
 */
int file_get_action(HDF *hdf, mdb_conn *conn, session_t *ses);

/*
 * get second-class nav menu
 */
int file_get_nav_by_id(mdb_conn *conn, int id, char *prefix, HDF *hdf);
int file_get_nav_by_uri(mdb_conn *conn, char *uri, char *prefix, HDF *hdf);


__END_DECLS
#endif /* __OFILE_H__ */
