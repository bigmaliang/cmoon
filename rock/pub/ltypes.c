#include "mheads.h"
#include "lheads.h"

file_t* file_new()
{
	file_t *fl = (file_t*)calloc(1, sizeof(file_t));
	if (fl == NULL)
		return NULL;
	
	return fl;
}
int file_pack(file_t *file, char **res, size_t *outlen)
{
	if (file == NULL || res == NULL) {
		return 1;
	}

	char *buf;
	int len = sizeof(file_t);
	len += strlen(file->name)+1;
	len += strlen(file->remark)+1;
	len += strlen(file->uri)+1;
	len += strlen(file->intime)+1;
	len += strlen(file->uptime)+1;

	buf = (char*)calloc(1, len);
	if (buf == NULL) {
		mtc_err("alloc mem for file pack failure");
		return 1;
	}
	memcpy(buf, file, sizeof(file_t));

	int pos = sizeof(file_t);
	memcpy(buf+pos, file->name, strlen(file->name)+1);

	pos += strlen(file->name)+1;
	memcpy(buf+pos, file->remark, strlen(file->remark)+1);

	pos += strlen(file->remark)+1;
	memcpy(buf+pos, file->uri, strlen(file->uri)+1);
	
	pos += strlen(file->uri)+1;
	memcpy(buf+pos, file->intime, strlen(file->intime)+1);

	pos += strlen(file->intime)+1;
	memcpy(buf+pos, file->uptime, strlen(file->uptime)+1);

	*res = buf;
	*outlen = len;
	
	return 0;
}
int file_unpack(char *buf, size_t inlen, file_t **file)
{
	if (inlen < sizeof(file_t)) {
		return 1;
	}

	char *p;
	file_t *fl = file_new();
	if (fl == NULL) {
		mtc_err("alloc mem for file unpack failure");
		return 1;
	}
#if 0
	fl = (file_t*)buf;
	fl->name = buf+sizeof(file_t);
	p = buf+sizeof(file_t);
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	fl->remark = p;
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	fl->intime = p;
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	fl->uptime = p;
#endif
	
	memcpy(fl, buf, sizeof(file_t));
	p = buf+sizeof(file_t);
	fl->name = strdup(p);
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	fl->remark = strdup(p);
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	fl->uri = strdup(p);
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	fl->intime = strdup(p);
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	fl->uptime = strdup(p);
	
	*file = fl;
	
	return 0;
}

void file_store_in_hdf(file_t *fl, char *prefix, HDF *hdf)
{
	if (fl == NULL || prefix == NULL)
		return;
	
	char prekey[LEN_ST], key[LEN_ST];
	snprintf(prekey, sizeof(prekey), "%s.%s", PRE_OUTPUT, prefix);

	snprintf(key, sizeof(key), "%s.id", prekey);
	hdf_set_int_value(hdf, key, fl->id);
	
	snprintf(key, sizeof(key), "%s.pid", prekey);
	hdf_set_int_value(hdf, key, fl->pid);
	
	snprintf(key, sizeof(key), "%s.uid", prekey);
	hdf_set_int_value(hdf, key, fl->uid);
	snprintf(key, sizeof(key), "%s.gid", prekey);
	hdf_set_int_value(hdf, key, fl->gid);
	snprintf(key, sizeof(key), "%s.mode", prekey);
	hdf_set_int_value(hdf, key, fl->mode);

	snprintf(key, sizeof(key), "%s.name", prekey);
	hdf_set_value(hdf, key, fl->name);
	snprintf(key, sizeof(key), "%s.remark", prekey);
	hdf_set_value(hdf, key, fl->remark);
	snprintf(key, sizeof(key), "%s.uri", prekey);
	hdf_set_value(hdf, key, fl->uri);
	snprintf(key, sizeof(key), "%s.intime", prekey);
	hdf_set_value(hdf, key, fl->intime);
	snprintf(key, sizeof(key), "%s.uptime", prekey);
	hdf_set_value(hdf, key, fl->uptime);
}

void file_del(void *fl)
{
	file_t *lfl = (file_t*)fl;

	if (lfl == NULL)
		return;

	if (lfl->name != NULL)
		free(lfl->name);
	if (lfl->remark != NULL)
		free(lfl->remark);
	if (lfl->uri != NULL)
		free(lfl->uri);
	if (lfl->intime != NULL)
		free(lfl->intime);
	if (lfl->uptime != NULL)
		free(lfl->uptime);
	if (lfl != NULL)
		free(fl);
}

member_t* member_new()
{
	member_t *mb = (member_t*)calloc(1, sizeof(member_t));
	if (mb == NULL)
		return NULL;
	
	return mb;
}
int member_pack(member_t *member, char **res, size_t *outlen)
{
	STRING infos;
	char *gid;
	NEOERR *err;
	int listlen = uListLength(member->gids);
	int i;

	if (member == NULL || res == NULL) {
		return 1;
	}

	string_init(&infos);
	for (i = 0; i < listlen; i++) {
		err = uListGet(member->gids, i, (void**)&gid);
		RETURN_V_NOK(err, 1);
		string_appendf(&infos, "%s:", gid);
	}
	
	char *buf;
	int len = sizeof(member_t);
	len += strlen(member->uname)+1;
	len += strlen(member->usn)+1;
	len += strlen(member->musn)+1;
	len += strlen(member->email)+1;
	len += strlen(member->intime)+1;
	len += strlen(member->uptime)+1;
	len += infos.len+1;

	buf = (char*)calloc(1, len);
	if (buf == NULL) {
		mtc_err("alloc mem for member pack failure");
		return 1;
	}
	memcpy(buf, member, sizeof(member_t));

	int pos = sizeof(member_t);
	memcpy(buf+pos, member->uname, strlen(member->uname)+1);

	pos += strlen(member->uname)+1;
	memcpy(buf+pos, member->usn, strlen(member->usn)+1);

	pos += strlen(member->usn)+1;
	memcpy(buf+pos, member->musn, strlen(member->musn)+1);

	pos += strlen(member->musn)+1;
	memcpy(buf+pos, member->email, strlen(member->email)+1);

	pos += strlen(member->email)+1;
	memcpy(buf+pos, member->intime, strlen(member->intime)+1);

	pos += strlen(member->intime)+1;
	memcpy(buf+pos, member->uptime, strlen(member->uptime)+1);
	
	pos += strlen(member->uptime)+1;
	memcpy(buf+pos, infos.buf, infos.len);
	*(buf+pos+1) = '\0';

	*res = buf;
	*outlen = len;

	string_clear(&infos);
	return 0;
}
int member_unpack(char *buf, size_t inlen, member_t **member)
{
	STRING infos;
	string_init(&infos);
	
	if (inlen < sizeof(member_t)) {
		return 1;
	}

	char *p;
	member_t *mb;
	NEOERR *err;
	mb = member_new();
	if (mb == NULL) {
		mtc_err("alloc mem for member unpack failure");
		return 1;
	}
	memcpy(mb, buf, sizeof(member_t));
	p = buf+sizeof(member_t);
	mb->uname = strdup(p);
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	mb->usn = strdup(p);
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	mb->musn = strdup(p);
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	mb->email = strdup(p);
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	mb->intime = strdup(p);
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	mb->uptime = strdup(p);
	while (*p != '\0' && p <= buf+inlen) p++; p++;
	string_append(&infos, p);
	err = string_array_split(&(mb->gids), infos.buf, ":", MAX_GROUP_AUSER);
	RETURN_V_NOK(err, 1);

	string_clear(&infos);
	*member = mb;
	
	return 0;
}
void member_del(void *mb)
{
	member_t *lmb = (member_t*)mb;

	if (lmb == NULL)
		return;
	if (lmb->uname != NULL)
		free(lmb->uname);
	if (lmb->usn != NULL)
		free(lmb->usn);
	if (lmb->musn != NULL)
		free(lmb->musn);
	if (lmb->email != NULL)
		free(lmb->email);
	if (lmb->intime != NULL)
		free(lmb->intime);
	if (lmb->uptime != NULL)
		free(lmb->uptime);
	if (lmb->gids != NULL)
		uListDestroy(&(lmb->gids), ULIST_FREE);
	if (lmb != NULL)
		free(lmb);
}
