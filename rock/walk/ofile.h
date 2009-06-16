#ifndef __OFILE_H__
#define __OFILE_H__
#include "mheads.h"

__BEGIN_DECLS

#define FILE_MASK_NOR	0x0000000F
#define FILE_MODE_NOR	0x00000000
#define FILE_MODE_DIR	0x00000001
#define FILE_MODE_LINK	0x00000002

/* 0001 0101 1111 0001 */
/* 5617 */
#define SYS_DIR_MODE	0x8AF1
/* 0001 0101 1111 0000 */
#define SYS_NOR_MODE	0x8AF0

#define PMS_OWNER(mode)	(mode>>4)
#define PMS_GROUP(mode)	(mode>>8)
#define PMS_OTHER(mode)	(mode>>12)

#define LMT_MASK	0x0000000F
#define LMT_GET		0x00000001
#define LMT_MOD		0x00000002
#define LMT_APPEND	0x00000004
#define LMT_DEL		0x00000008

int file_check_user_power(CGI *cgi, mdb_conn *conn, file_t *file, int access);
int file_get_info(mdb_conn *conn, int id, char *url, int pid, file_t **file);
int file_get_infos(mdb_conn *conn, ULIST *urls, ULIST **files, int *noksn);
int file_get_info_uri(mdb_conn *conn, char *uri, file_t **file);
void file_refresh_info(mdb_conn *conn, int id, char *url, int pid);

/*
 * get files user can manage: add, mod, del 
 */
int  file_get_files(HDF *hdf, mdb_conn *conn);
void file_translate_mode(HDF *hdf);
int  file_modify(HDF *hdf, mdb_conn *conn);
int  file_add(HDF *hdf, mdb_conn *conn);
int  file_delete(HDF *hdf, mdb_conn *conn);

/*
 * get logined user's action
 */
int file_get_action(HDF *hdf, mdb_conn *conn);

/*
 * get second-class nav menu
 */
int file_get_nav(HDF *hdf, mdb_conn *conn);


__END_DECLS
#endif /* __OFILE_H__ */
