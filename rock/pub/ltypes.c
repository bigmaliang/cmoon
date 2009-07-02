#include "mheads.h"
#include "lheads.h"

/*
 * MACROS LOCALY USED
 */
#define STRUCT_ADD_LEN(len, parent, name)		\
	do {										\
		if (parent->name != NULL)				\
			len += strlen(parent->name);		\
		len += 1;								\
	} while (0)

#define STRUCT_PACK_INT(parent, namesrc)				\
	do {												\
		*(int*)(buf+pos) = parent->namesrc;				\
		pos += sizeof(int);								\
	} while (0)

#define STRUCT_PACK_STR(parent, namesrc)								\
	do {																\
		if (parent->namesrc != NULL) { 									\
			memcpy(buf+pos, parent->namesrc, strlen(parent->namesrc)+1); \
			pos += strlen(parent->namesrc)+1;							\
		} else {														\
			*(buf+pos) = '\0';											\
			pos += 1;													\
		}																\
	} while (0)

#define STRUCT_UNPACK_INT(psrc, parent, namedest)	\
	do {											\
		parent->namedest = *(int*)psrc;				\
		psrc += sizeof(int);						\
	} while (0)

#define STRUCT_UNPACK_STR(psrc, parent, namedest)				\
	do {														\
		if (*psrc != '\0') {									\
			parent->namedest = strdup(psrc);					\
			while (*psrc != '\0' && psrc <= buf+inlen) psrc++;	\
		}														\
		psrc++;													\
	} while (0)

#define STORE_IN_HDF_INT(parent, dstname)						\
	do {														\
		snprintf(key, sizeof(key), "%s.%s", prefix, #dstname);	\
		hdf_set_int_value(hdf, key, parent->dstname);			\
	} while (0)

#define STORE_IN_HDF_STR(parent, dstname)						\
	do {														\
		snprintf(key, sizeof(key), "%s.%s", prefix, #dstname);	\
		hdf_set_value(hdf, key, parent->dstname);				\
	} while (0)

#define SAFE_FREE(str)							\
	do {										\
		if (str != NULL)						\
			free(str);							\
	} while (0)


/*
 * public tool func
 */
size_t list_pack(ULIST *list, size_t (*pack)(void *item, char *buf), char *buf)
{
	size_t pos = 0;
	void *node;

	if (list == NULL || pack == NULL || buf == NULL)
		return 0;

	STRUCT_PACK_INT(list, flags);
	STRUCT_PACK_INT(list, num);
	STRUCT_PACK_INT(list, max);
	MLIST_ITERATE(list, node) {
		pos += pack(node, buf+pos);
	}

	return pos;
}

char* list_unpack(char *buf, size_t (*unpack)(char *buf, void **item), ULIST *list)
{
	void *node;
	
	if (list == NULL || unpack == NULL || buf == NULL)
		return buf;

	STRUCT_UNPACK_INT(buf, list, flags);
	STRUCT_UNPACK_INT(buf, list, num);
	STRUCT_UNPACK_INT(buf, list, max);

	for (int i = 0; i <  list->num; i++) {
		buf += unpack(buf, &node);
		uListSet(list, i, node);
	}
	
	return buf;
}


/*
 * file 
 */
file_t* file_new()
{
	file_t *fl = (file_t*)calloc(1, sizeof(file_t));
	if (fl == NULL)
		return NULL;
	
	return fl;
}

int file_pack(file_t *file, char **res, size_t *outlen)
{
	*res = NULL;
	if (file == NULL || res == NULL) {
		return RET_RBTOP_INPUTE;
	}

	char *buf;
	size_t len = sizeof(file_t);

	STRUCT_ADD_LEN(len, file, name);
	STRUCT_ADD_LEN(len, file, remark);
	STRUCT_ADD_LEN(len, file, uri);
	STRUCT_ADD_LEN(len, file, dataer);
	STRUCT_ADD_LEN(len, file, render);
	STRUCT_ADD_LEN(len, file, intime);
	STRUCT_ADD_LEN(len, file, uptime);
	len += 1;

	buf = (char*)calloc(1, len);
	if (buf == NULL) {
		mtc_err("alloc mem for file pack failure");
		return RET_RBTOP_MEMALLOCE;
	}

	memcpy(buf, file, sizeof(file_t));
	size_t pos = sizeof(file_t);

	STRUCT_PACK_STR(file, name);
	STRUCT_PACK_STR(file, remark);
	STRUCT_PACK_STR(file, uri);
	STRUCT_PACK_STR(file, dataer);
	STRUCT_PACK_STR(file, render);
	STRUCT_PACK_STR(file, intime);
	STRUCT_PACK_STR(file, uptime);

	*res = buf;
	*outlen = len;
	
	return RET_RBTOP_OK;
}

int file_unpack(char *buf, size_t inlen, file_t **file)
{
	if (inlen < sizeof(file_t)) {
		return RET_RBTOP_INPUTE;
	}

	char *p;
	file_t *fl = file_new();
	if (fl == NULL) {
		mtc_err("alloc mem for file unpack failure");
		return RET_RBTOP_MEMALLOCE;
	}

	memcpy(fl, buf, sizeof(file_t));
	p = buf+sizeof(file_t);

	STRUCT_UNPACK_STR(p, fl, name);
	STRUCT_UNPACK_STR(p, fl, remark);
	STRUCT_UNPACK_STR(p, fl, uri);
	STRUCT_UNPACK_STR(p, fl, dataer);
	STRUCT_UNPACK_STR(p, fl, render);
	STRUCT_UNPACK_STR(p, fl, intime);
	STRUCT_UNPACK_STR(p, fl, uptime);
	
	*file = fl;
	
	return RET_RBTOP_OK;
}

void file_store_in_hdf(file_t *fl, char *prefix, HDF *hdf)
{
	if (fl == NULL || prefix == NULL || hdf == NULL)
		return;
	
	char key[LEN_ST];
	
	STORE_IN_HDF_INT(fl, id);
	STORE_IN_HDF_INT(fl, pid);
	STORE_IN_HDF_INT(fl, uid);
	STORE_IN_HDF_INT(fl, gid);
	STORE_IN_HDF_INT(fl, mode);
	STORE_IN_HDF_INT(fl, reqtype);
	STORE_IN_HDF_INT(fl, lmttype);
	STORE_IN_HDF_STR(fl, name);
	STORE_IN_HDF_STR(fl, remark);
	STORE_IN_HDF_STR(fl, uri);
	STORE_IN_HDF_STR(fl, dataer);
	STORE_IN_HDF_STR(fl, render);
	STORE_IN_HDF_STR(fl, intime);
	STORE_IN_HDF_STR(fl, uptime);
}

void file_reset(file_t *fl)
{
	if (fl == NULL)
		return;

	SAFE_FREE(fl->name);
	SAFE_FREE(fl->remark);
	SAFE_FREE(fl->uri);
	SAFE_FREE(fl->dataer);
	SAFE_FREE(fl->render);
	SAFE_FREE(fl->intime);
	SAFE_FREE(fl->uptime);
}

void file_del(void *fl)
{
	file_t *lfl = (file_t*)fl;

	if (lfl == NULL)
		return;

	file_reset(lfl);
	free(fl);
}


/*
 * member
 */
member_t* member_new()
{
	member_t *mb = (member_t*)calloc(1, sizeof(member_t));
	if (mb == NULL)
		return NULL;
	
	return mb;
}

int member_pack(member_t *member, char **res, size_t *outlen)
{
	*res = NULL;
	if (member == NULL || res == NULL) {
		return RET_RBTOP_INPUTE;
	}

	char *buf;
	size_t len = sizeof(member_t);

	STRUCT_ADD_LEN(len, member, uname);
	STRUCT_ADD_LEN(len, member, usn);
	STRUCT_ADD_LEN(len, member, musn);
	STRUCT_ADD_LEN(len, member, email);
	STRUCT_ADD_LEN(len, member, intime);
	STRUCT_ADD_LEN(len, member, uptime);
	STRUCT_ADD_LEN(len, member, gids);
	STRUCT_ADD_LEN(len, member, gmodes);
	len += 1;

	buf = (char*)calloc(1, len);
	if (buf == NULL) {
		mtc_err("alloc mem for member pack failure");
		return RET_RBTOP_MEMALLOCE;
	}

	memcpy(buf, member, sizeof(member_t));
	size_t pos = sizeof(member_t);
	STRUCT_PACK_STR(member, uname);
	STRUCT_PACK_STR(member, usn);
	STRUCT_PACK_STR(member, musn);
	STRUCT_PACK_STR(member, email);
	STRUCT_PACK_STR(member, intime);
	STRUCT_PACK_STR(member, uptime);
	STRUCT_PACK_STR(member, gids);
	STRUCT_PACK_STR(member, gmodes);
	/*
	 * oh, fuck, this line will cause SIGABORT (double free)
	 */
	//*(buf+pos+1) = '\0';

	*res = buf;
	*outlen = len;

	return RET_RBTOP_OK;
}

int member_unpack(char *buf, size_t inlen, member_t **member)
{
	STRING str;
	string_init(&str);
	
	if (inlen < sizeof(member_t)) {
		return RET_RBTOP_INPUTE;
	}

	char *p;
	member_t *mb;
	mb = member_new();
	if (mb == NULL) {
		mtc_err("alloc mem for member unpack failure");
		return RET_RBTOP_MEMALLOCE;
	}
	memcpy(mb, buf, sizeof(member_t));
	p = buf+sizeof(member_t);
	
	STRUCT_UNPACK_STR(p, mb, uname);
	STRUCT_UNPACK_STR(p, mb, usn);
	STRUCT_UNPACK_STR(p, mb, musn);
	STRUCT_UNPACK_STR(p, mb, email);
	STRUCT_UNPACK_STR(p, mb, intime);
	STRUCT_UNPACK_STR(p, mb, uptime);
	STRUCT_UNPACK_STR(p, mb, gids);
	STRUCT_UNPACK_STR(p, mb, gmodes);
	
	*member = mb;
	
	return RET_RBTOP_OK;
}

void member_del(void *mb)
{
	member_t *lmb = (member_t*)mb;

	if (lmb == NULL)
		return;

	SAFE_FREE(lmb->uname);
	SAFE_FREE(lmb->usn);
	SAFE_FREE(lmb->musn);
	SAFE_FREE(lmb->email);
	SAFE_FREE(lmb->intime);
	SAFE_FREE(lmb->uptime);
	SAFE_FREE(lmb->gids);
	SAFE_FREE(lmb->gmodes);

	free(lmb);
}


/*
 * gnode
 */
#define GNODE_LEN	(sizeof(gnode_t))

gnode_t* gnode_new()
{
	return (gnode_t*)calloc(1, sizeof(gnode_t));
}

size_t gnode_pack_nalloc(void *node, char *buf)
{
	memcpy(buf, node, sizeof(gnode_t));
	return GNODE_LEN;
}

size_t gnode_unpack(char *buf, void **gnode)
{
	*gnode = NULL;
	gnode_t *gn = gnode_new();
	if (gn == NULL) {
		mtc_err("alloc mem for gnode unpack failure");
		return 0;
	}
	memcpy(gn, buf, sizeof(gnode_t));
	*gnode = (void*)gn;
	return GNODE_LEN;
}

void gnode_del(void *gn)
{
	gnode_t *lgn = (gnode_t*)gn;
	if (lgn == NULL)
		return;
	free(lgn);
}


/*
 * group
 */
group_t* group_new()
{
	NEOERR *err;
	group_t *gp = (group_t*)calloc(1, sizeof(group_t));
	if (gp == NULL)
		return NULL;
	err = uListInit(&(gp->node) , 0, 0);
	/* TODO memroy leak */
	RETURN_V_NOK(err, NULL);

	return gp;
}

int group_pack(group_t *gp, char **res, size_t *outlen)
{
	*res = NULL;
	if (gp == NULL || res == NULL) {
		return RET_RBTOP_INPUTE;
	}

	char *buf;
	size_t len = sizeof(group_t);
	len += sizeof(ULIST);
	len += uListLength(gp->node) * GNODE_LEN;
	len += 1;
	
	buf = (char*)calloc(1, len);
	if (buf == NULL) {
		mtc_err("calloc mem for group pack failure");
		return RET_RBTOP_MEMALLOCE;
	}

	memcpy(buf, gp, sizeof(gp));
	size_t pos = sizeof(group_t);

	pos += list_pack(gp->node, gnode_pack_nalloc, buf);

	*res = buf;
	*outlen = len;
	return RET_RBTOP_OK;
}

int group_unpack(char *buf, size_t inlen, group_t **group)
{
	if (inlen < sizeof(group_t) + sizeof(ULIST)) {
		return RET_RBTOP_INPUTE;
	}

	char *p;
	group_t *gp = group_new();
	if (gp == NULL) {
		mtc_err("alloc mem for group unpack failure");
		return RET_RBTOP_MEMALLOCE;
	}

	gp->gid = *(int*)buf;
	p = buf + sizeof(group_t);

	p = list_unpack(p, gnode_unpack, gp->node);

	*group = gp;

	return RET_RBTOP_OK;
}

void group_store_in_hdf(group_t *gp, char *prefix, HDF *hdf)
{
	if (gp == NULL || prefix == NULL || hdf == NULL)
		return;

	char key[LEN_ST], tok[LEN_MD];

	STORE_IN_HDF_INT(gp, gid);

	gnode_t *node;
	if (gp->node != NULL) {
		MLIST_ITERATE(gp->node, node) {
			sprintf(tok, "%s.members.%d", prefix, t_rsv_i);
			prefix = tok;
			STORE_IN_HDF_INT(node, uid);
			STORE_IN_HDF_INT(node, gid);
			STORE_IN_HDF_INT(node, mode);
			STORE_IN_HDF_INT(node, status);
		}
	}
}

void group_del(void *gp)
{
	group_t *lgp = (group_t*)gp;

	if (lgp == NULL)
		return;
	if (lgp->node != NULL)
		uListDestroyFunc(&lgp->node, gnode_del);
	free(lgp);
}


/*
 * session
 */
#include "omember.h"

int session_init(HDF *hdf, HASH *dbh, session_t **ses)
{
	int ret;
	session_t *lses;
	
	*ses = NULL;
	
	lses = calloc(1, sizeof(session_t));
	if (lses == NULL) {
		mtc_err("calloc memory for session_t failure");
		return RET_RBTOP_MEMALLOCE;
	}

	*ses = lses;

	int uin = hdf_get_int_value(hdf, PRE_COOKIE".uin", -1);
	ret = member_get_info((mdb_conn*)hash_lookup(dbh, "Sys"), uin, &(lses->member));
	if (ret != RET_RBTOP_OK) {
		mtc_info("get %d member info failure", uin);
		/*
		 * set guest info avoid sigsegv 
		 */
		lses->member = member_new();
		if (lses->member == NULL) {
			mtc_err("alloc guest info failure");
			return RET_RBTOP_MEMALLOCE;
		}
		lses->member->uin = 0;
		lses->member->male = 0;
		lses->member->status = 0;
		lses->member->uname = "guest";
		lses->member->musn = "nothing";
		lses->member->email = "nothing";
		lses->member->intime = "1970-01-01";
		lses->member->uptime = "1970-01-01";
		lses->member->gids = "-1";
		lses->member->gmodes = "-1";
	}

	return RET_RBTOP_OK;
}

void session_destroy(session_t **ses)
{
	session_t *lses;
	if (ses == NULL) return;
	lses = *ses;

	if (lses == NULL) return;

	if (lses->member != NULL)
		member_del(lses->member);
	if (lses->file != NULL)
		file_del(lses->file);

	free(lses);
	lses = NULL;
}
