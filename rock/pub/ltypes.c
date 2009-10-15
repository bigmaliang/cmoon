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
size_t list_pack_nalloc(ULIST *list, size_t (*pack)(void *item, char *buf),
                        char *buf)
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

int list_pack(ULIST *ul, size_t (*item_len)(void *item),
              size_t (*pack)(void *item, char *buf),
              char **res, size_t *outlen)
{
    *res = NULL;
    if (ul == NULL || item_len == NULL || pack == NULL)
        return RET_RBTOP_INPUTE;
    
    char *buf;
    size_t len = sizeof(ULIST);


    ITERATE_MLIST(ul) {
        len += item_len(ul->items[t_rsv_i]);
    }
    len += 1;

    buf = (char*)calloc(1, len);
    if (buf == NULL) {
        mtc_err("alloc mem for list pack failure");
        return RET_RBTOP_MEMALLOCE;
    }

    list_pack_nalloc(ul, pack, buf);

	*res = buf;
	*outlen = len;

    return RET_RBTOP_OK;
}

char* list_unpack(char *buf,
                  int (*unpack)(char *buf, size_t ilen, void **item, size_t *olen),
				  size_t inlen, ULIST **ul)
{
	void *node;
	size_t len = 0, reslen = 0;
	
	if (ul == NULL || unpack == NULL || buf == NULL)
		return buf;

    if (*ul == NULL) {
        uListInit(ul, 0, 0);
    }

    ULIST *list = *ul;

	STRUCT_UNPACK_INT(buf, list, flags);
	STRUCT_UNPACK_INT(buf, list, num);
	STRUCT_UNPACK_INT(buf, list, max);

	len = sizeof(int)*3;

	for (int i = 0; i <  list->num; i++) {
		if (len >= inlen)
			break;
		if (unpack(buf, inlen-len, &node, &reslen) == RET_RBTOP_OK) {
            buf += reslen;
            len += reslen;
            uListSet(list, i, node);
        } else {
            break;
        }
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

int file_unpack(char *buf, size_t inlen, file_t **file, size_t *outlen)
{
    *file = NULL;
    if (outlen != NULL) *outlen = 0;
    
	if (inlen < sizeof(file_t) || buf == NULL) {
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
    if (outlen != NULL) *outlen = p - buf;
	
	return RET_RBTOP_OK;
}

void file_item2hdf(file_t *fl, char *prefix, HDF *hdf)
{
	if (fl == NULL || prefix == NULL || hdf == NULL)
		return;
	
	char key[LEN_ST];
	
	STORE_IN_HDF_INT(fl, id);
	STORE_IN_HDF_INT(fl, aid);
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
	NEOERR *err;
	member_t *mb = (member_t*)calloc(1, sizeof(member_t));
	if (mb == NULL)
		return NULL;
	err = uListInit(&(mb->gnode), 0, 0);
	/* TODO memory leak */
	RETURN_V_NOK(err, NULL);
	
	return mb;
}

int member_pack(member_t *member, char **res, size_t *outlen)
{
	*res = NULL;
	if (member == NULL || res == NULL) {
		return RET_RBTOP_INPUTE;
	}

	char *buf;
	gnode_t *node;
	size_t len = sizeof(member_t);

	STRUCT_ADD_LEN(len, member, uname);
	STRUCT_ADD_LEN(len, member, usn);
	STRUCT_ADD_LEN(len, member, musn);
	STRUCT_ADD_LEN(len, member, email);
	STRUCT_ADD_LEN(len, member, intime);
	STRUCT_ADD_LEN(len, member, uptime);
	MLIST_ITERATE(member->gnode, node) {
		len += GNODE_LEN(node);
	}
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
	pos += list_pack_nalloc(member->gnode, gnode_pack_nalloc, buf);
	/*
	 * oh, fuck, this line will cause SIGABORT (double free)
	 */
	//*(buf+pos+1) = '\0';

	*res = buf;
	*outlen = len;

	return RET_RBTOP_OK;
}

int member_unpack(char *buf, size_t inlen, member_t **member, size_t *outlen)
{
    *member = NULL;
    if (outlen != NULL) *outlen = 0;
    
	STRING str;
	string_init(&str);
	
	if (inlen < sizeof(member_t) || buf == NULL) {
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

	p = list_unpack(p, gnode_unpack, inlen-(p-buf), &(mb->gnode));
	
	*member = mb;
    if (outlen != NULL) *outlen = p - buf;
	
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
	if (lmb->gnode != NULL)
		uListDestroyFunc(&lmb->gnode, gnode_del);
	free(lmb);
}


/*
 * gnode
 */
size_t GNODE_LEN(gnode_t *node)
{
	if (node == NULL)
		return 0;

	size_t len = sizeof(gnode_t);
	
	if (node->intime != NULL)
		len += strlen(node->intime) + 1;
	if (node->uptime != NULL)
		len += strlen(node->uptime) + 1;

	return len;
}

gnode_t* gnode_new()
{
	return (gnode_t*)calloc(1, sizeof(gnode_t));
}

size_t gnode_pack_nalloc(void *node, char *buf)
{
	size_t pos = 0;
	gnode_t *gn = (gnode_t*)node;
	
	memcpy(buf, node, sizeof(gnode_t));
	pos += sizeof(gnode_t);

	STRUCT_PACK_STR(gn, intime);
	STRUCT_PACK_STR(gn, uptime);
	
	return pos;
}

int gnode_unpack(char *buf, size_t inlen, void **gnode, size_t *outlen)
{
	*(gnode_t**)gnode = NULL;
    if (outlen != NULL) *outlen = 0;
	
	if (inlen < sizeof(gnode_t) || buf == NULL) {
		return RET_RBTOP_INPUTE;
	}
    
	char *p;
	gnode_t *gn = gnode_new();
	if (gn == NULL) {
		mtc_err("alloc mem for gnode unpack failure");
		return RET_RBTOP_MEMALLOCE;
	}
	memcpy(gn, buf, sizeof(gnode_t));

	p = buf + sizeof(gnode_t);

	STRUCT_UNPACK_STR(p, gn, intime);
	STRUCT_UNPACK_STR(p, gn, uptime);
	
	*gnode = (void*)gn;
    if (outlen != NULL) *outlen = p - buf;
    
	return RET_RBTOP_OK;
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
	gnode_t *node;
	size_t len = sizeof(group_t);
	len += sizeof(ULIST);
	MLIST_ITERATE(gp->node, node) {
		len += GNODE_LEN(node);
	}
	len += 1;
	
	buf = (char*)calloc(1, len);
	if (buf == NULL) {
		mtc_err("calloc mem for group pack failure");
		return RET_RBTOP_MEMALLOCE;
	}

	memcpy(buf, gp, sizeof(gp));
	size_t pos = sizeof(group_t);

	pos += list_pack_nalloc(gp->node, gnode_pack_nalloc, buf);

	*res = buf;
	*outlen = len;
	return RET_RBTOP_OK;
}

int group_unpack(char *buf, size_t inlen, group_t **group, size_t *outlen)
{
    *group = NULL;
    if (outlen != NULL) *outlen = 0;
    
	if (inlen < sizeof(group_t) + sizeof(ULIST) || buf == NULL) {
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

	p = list_unpack(p, gnode_unpack, inlen-(p-buf), &(gp->node));

	*group = gp;
    if (outlen != NULL) *outlen = p - buf;

	return RET_RBTOP_OK;
}

void group_item2hdf(group_t *gp, char *prekey, HDF *hdf)
{
	if (gp == NULL || prekey == NULL || hdf == NULL)
		return;

	char key[LEN_ST], tok[LEN_MD], *prefix = prekey;

	STORE_IN_HDF_INT(gp, gid);

	gnode_t *node;
	if (gp->node != NULL) {
		MLIST_ITERATE(gp->node, node) {
			sprintf(tok, "%s.members.%d", prekey, t_rsv_i);
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
 * tjt
 */
size_t TJT_LEN(void *item)
{
    if (item == NULL)
        return 0;

    tjt_t *lt = item;
    size_t len = sizeof(tjt_t);

    if (lt->img != NULL)
        len += strlen(lt->img) + 1;
    if (lt->imgurl != NULL)
        len += strlen(lt->imgurl) + 1;
    if (lt->exp != NULL)
        len += strlen(lt->exp) + 1;
    if (lt->intime != NULL)
        len += strlen(lt->intime) + 1;
    if (lt->uptime != NULL)
        len += strlen(lt->uptime) + 1;

    return len;
}

tjt_t* tjt_new()
{
    return (tjt_t*)calloc(1, sizeof(tjt_t));
}

int tjt_pack(tjt_t *tjt, char **res, size_t *outlen)
{
	*res = NULL;
	if (tjt == NULL || res == NULL) {
		return RET_RBTOP_INPUTE;
	}

	char *buf;
	size_t len = sizeof(tjt_t);

	STRUCT_ADD_LEN(len, tjt, img);
	STRUCT_ADD_LEN(len, tjt, imgurl);
	STRUCT_ADD_LEN(len, tjt, exp);
	STRUCT_ADD_LEN(len, tjt, intime);
	STRUCT_ADD_LEN(len, tjt, uptime);
	len += 1;

	buf = (char*)calloc(1, len);
	if (buf == NULL) {
		mtc_err("alloc mem for tjt pack failure");
		return RET_RBTOP_MEMALLOCE;
	}

	memcpy(buf, tjt, sizeof(tjt_t));
	size_t pos = sizeof(tjt_t);

	STRUCT_PACK_STR(tjt, img);
	STRUCT_PACK_STR(tjt, imgurl);
	STRUCT_PACK_STR(tjt, exp);
	STRUCT_PACK_STR(tjt, intime);
	STRUCT_PACK_STR(tjt, uptime);

	*res = buf;
	*outlen = len;
	
	return RET_RBTOP_OK;
}

size_t tjt_pack_nalloc(void *tjt, char *buf)
{
	size_t pos = 0;
	tjt_t *lt = (tjt_t*)tjt;
	
	memcpy(buf, tjt, sizeof(tjt_t));
	pos += sizeof(tjt_t);

	STRUCT_PACK_STR(lt, img);
	STRUCT_PACK_STR(lt, imgurl);
	STRUCT_PACK_STR(lt, exp);
	STRUCT_PACK_STR(lt, intime);
	STRUCT_PACK_STR(lt, uptime);
	
	return pos;
}

int tjt_unpack(char *buf, size_t inlen, void **tjt, size_t *outlen)
{
    *(tjt_t**)tjt = NULL;
    if (outlen != NULL) *outlen = 0;
    
	if (inlen < sizeof(tjt_t) || buf == NULL) {
		return RET_RBTOP_INPUTE;
	}

	char *p;
	tjt_t *lt = tjt_new();
	if (lt == NULL) {
		mtc_err("alloc mem for tjt unpack failure");
		return RET_RBTOP_MEMALLOCE;
	}

	memcpy(lt, buf, sizeof(tjt_t));
	p = buf+sizeof(tjt_t);

	STRUCT_UNPACK_STR(p, lt, img);
	STRUCT_UNPACK_STR(p, lt, imgurl);
	STRUCT_UNPACK_STR(p, lt, exp);
	STRUCT_UNPACK_STR(p, lt, intime);
	STRUCT_UNPACK_STR(p, lt, uptime);
	
	*(tjt_t**)tjt = lt;
    if (outlen != NULL) *outlen = p - buf;
	
	return RET_RBTOP_OK;
}

void tjt_hdf2item(HDF *hdf, void **tjt)
{
    *tjt = NULL;

    tjt_t *lt = tjt_new();
    if (lt == NULL) {
        mtc_err("alloc mem for tjt failure");
        return;
    }

    lt->id = hdf_get_int_value(hdf, "id", 0);
    lt->aid = hdf_get_int_value(hdf, "aid", 0);
    lt->fid = hdf_get_int_value(hdf, "fid", 0);
    lt->uid = hdf_get_int_value(hdf, "uid", 0);
	/*
	 * hdf_get_value here will cause sigv
	 * because we'll free lt later
	 * so, use hdf_get_copy
	 */
	hdf_get_copy(hdf, "img", &lt->img, NULL);
    hdf_get_copy(hdf, "imgurl", &lt->imgurl, NULL);
    hdf_get_copy(hdf, "exp", &lt->exp, NULL);
    hdf_get_copy(hdf, "intime", &lt->intime, NULL);
    hdf_get_copy(hdf, "uptime", &lt->uptime, NULL);

    *tjt = lt;
}

void tjt_item2hdf(void *tjt, char *prefix, HDF *hdf)
{
	if (tjt == NULL || prefix == NULL || hdf == NULL)
		return;
	
	char key[LEN_ST];
    tjt_t *lt = tjt;
	
	STORE_IN_HDF_INT(lt, id);
	STORE_IN_HDF_INT(lt, aid);
	STORE_IN_HDF_INT(lt, fid);
	STORE_IN_HDF_INT(lt, uid);
	STORE_IN_HDF_STR(lt, img);
	STORE_IN_HDF_STR(lt, imgurl);
	STORE_IN_HDF_STR(lt, exp);
	STORE_IN_HDF_STR(lt, intime);
	STORE_IN_HDF_STR(lt, uptime);
}

void tjt_del(void *tjt)
{
	tjt_t *lt = (tjt_t*)tjt;

	if (lt == NULL)
		return;

	SAFE_FREE(lt->img);
	SAFE_FREE(lt->imgurl);
	SAFE_FREE(lt->exp);
	SAFE_FREE(lt->intime);
	SAFE_FREE(lt->uptime);
	free(lt);
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
		lses->member->uname = strdup("guest");
		lses->member->musn = strdup("nothing");
		lses->member->email = strdup("nothing");
		lses->member->intime = strdup("1970-01-01");
		lses->member->uptime = strdup("1970-01-01");
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
