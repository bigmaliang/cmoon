#ifndef __LTYPES_H__
#define __LTYPES_H__

#include "mheads.h"

__BEGIN_DECLS

typedef struct _file {
	int id;
	int pid;
	int uid;
	int gid;
	int mode;					/* file mode */
	int reqtype;				/* file request type */
	int lmttype;				/* some extra limit to access this file */
	char *name;
	char *remark;
	char *uri;
	char *dataer;				/* data handler plugin name */
	char *render;				/* html rend plugin name */
	char *intime;
	char *uptime;
} file_t;

typedef struct _member {
	int uin;
	int male;
	int status;
	char *uname;
	char *usn;
	char *musn;
	char *email;
	char *intime;
	char *uptime;
	char *gids;
	char *gmodes;
} member_t;

typedef struct _gnode {
	int uid;
	int mode;
	int stat;
} gnode_t;
typedef struct _group {
	int gid;
	ULIST *node;
} group_t;

file_t* file_new();
int  file_pack(file_t *file, char **res, size_t *outlen);
int  file_unpack(char *buf, size_t inlen, file_t **file);
void file_store_in_hdf(file_t *fl, char *prefix, HDF *hdf);
void file_reset(file_t *fl);
void file_del(void *file);

member_t* member_new();
int  member_pack(member_t *member, char **res, size_t *outlen);
int  member_unpack(char *buf, size_t inlen, member_t **member);
void member_del(void *member);

gnode_t* gnode_new();
size_t gnode_pack_nalloc(gnode_t *node, char *buf);
size_t gnode_unpack(char *buf, gnode_t **gnode);
void   gnode_del(void *gn);

group_t* group_new();
int  group_pack(group_t *group, char **res, size_t *outlen);
int  group_unpack(char *buf, size_t inlen, group_t **group);
void group_store_in_hdf(group_t *fl, char *prefix, HDF *hdf);
void group_del(void *group);

typedef struct _session {
	member_t *member;
	file_t *file;
	time_t tm_cache_browser;
} session_t;

int session_init(HDF *hdf, HASH *dbh, session_t **ses);
void session_destroy(session_t **ses);

__END_DECLS
#endif	/* __LTYPES_H__ */
