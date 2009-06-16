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
	int mask;					/* file type mask */
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
	ULIST *gids;
} member_t;

file_t* file_new();
int  file_pack(file_t *file, char **res, size_t *outlen);
int  file_unpack(char *buf, size_t inlen, file_t **file);
void file_store_in_hdf(file_t *fl, char *prefix, HDF *hdf);
void file_del(void *file);

member_t* member_new();
int  member_pack(member_t *member, char **res, size_t *outlen);
int  member_unpack(char *buf, size_t inlen, member_t **member);
void member_del(void *member);

__END_DECLS
#endif	/* __LTYPES_H__ */
