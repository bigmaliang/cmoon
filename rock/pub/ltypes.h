#ifndef __LTYPES_H__
#define __LTYPES_H__

#include "mheads.h"

__BEGIN_DECLS

typedef struct _gnode {
	int uid;
	int gid;
	int mode;
	int status;
	char *intime;
	char *uptime;
} gnode_t;

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
	ULIST *gnode;
} member_t;

typedef struct _group {
	int gid;
	ULIST *node;
} group_t;

typedef struct _tjt {
	int id;
	int fid;
	int uid;
	char *img;
	char *exp;
	char *intime;
	char *uptime;
} tjt_t;

/* memory alloced already, e.g. in member's gnode */
size_t list_pack_nalloc(ULIST *list,
                        size_t (*pack)(void *item, char *buf),
                        char *buf);
/* top level pack, e.g. in tjt's list */
int list_pack(ULIST *ul, size_t (*item_len)(void *item),
              size_t (*pack)(void *item, char *buf),
              char **res, size_t *outlen);
/* if *ul == NULL, i'll init it */
char* list_unpack(char *buf, size_t (*unpack)(char *buf, size_t inlen, void **item),
				  size_t inlen, ULIST **ul);

file_t* file_new();
int  file_pack(file_t *file, char **res, size_t *outlen);
int  file_unpack(char *buf, size_t inlen, file_t **file);
void file_item2hdf(file_t *fl, char *prefix, HDF *hdf);
void file_reset(file_t *fl);
void file_del(void *file);

member_t* member_new();
int  member_pack(member_t *member, char **res, size_t *outlen);
int  member_unpack(char *buf, size_t inlen, member_t **member);
void member_del(void *member);

size_t GNODE_LEN(gnode_t *node);
gnode_t* gnode_new();
size_t gnode_pack_nalloc(void *node, char *buf);
size_t gnode_unpack(char *buf, size_t inlen, void **gnode);
void   gnode_del(void *gn);

group_t* group_new();
int  group_pack(group_t *group, char **res, size_t *outlen);
int  group_unpack(char *buf, size_t inlen, group_t **group);
void group_item2hdf(group_t *fl, char *prefix, HDF *hdf);
void group_del(void *group);

size_t TJT_LEN(void *node);
tjt_t* tjt_new();
/* top level pack, not in list */
int tjt_pack(tjt_t *tjt, char **res, size_t *outlen);
/* memory alloced already, e.g. in list mode */
size_t tjt_pack_nalloc(void *tjt, char *buf);
int tjt_unpack(char *buf, size_t inlen, void **tjt);
void tjt_hdf2item(HDF *hdf, void **tjt);
void tjt_item2hdf(void *tjt, char *prefix, HDF *hdf);
void tjt_del(void *tjt);

typedef struct _session {
	member_t *member;
	file_t *file;
	time_t tm_cache_browser;
} session_t;

int session_init(HDF *hdf, HASH *dbh, session_t **ses);
void session_destroy(session_t **ses);

__END_DECLS
#endif	/* __LTYPES_H__ */
