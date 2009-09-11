#include "mevent_plugin.h"
#include "mevent_uic.h"

#define PLUGIN_NAME	"uic"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct uic_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct uic_entry {
	struct event_entry base;
	FILE *logf;
	fdb_t *db;
	struct cache *cd;
	struct uic_stats st;
};

/*
 * input : uin(UINT)
 * return: NORMAL
 * reply : ["friend": [ "100": 2, "200": 3, ...]] OR ["friend":[]]
 */
static int uic_cmd_friendlist(struct queue_entry *q, struct cache *cd,
							  fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	unsigned char *val = NULL;
	size_t vsize = 0;
	int uin, hit, ret;
	
	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "uin");
	if (c != NULL) {
		uin = c->v.ival;
		reply_add_array(q, NULL, "friend");
		hit = cache_getf(cd, &val, &vsize, PREFIX_FRIEND"%d", uin);
		if (hit == 0) {
			snprintf(db->sql, sizeof(db->sql), "SELECT friend_userid, groupid FROM "
					 " user_friends WHERE userid=%d AND beok=1;", uin);
			ret = fdb_exec(db);
			if (ret != RET_DBOP_OK) {
				dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
				return REP_ERR_DB;
			}
			while (fdb_fetch_row(db) == RET_DBOP_OK) {
				reply_add_u32(q, "friend", db->row[0], atoi(db->row[1]));
			}
			val = calloc(1, MAX_PACKET_LEN);
			if (val == NULL) {
				return REP_ERR_MEM;
			}
			vsize = pack_data_array(NULL, q->replydata, val,
									MAX_PACKET_LEN - RESERVE_SIZE);
			if (vsize == 0) {
				free(val);
				return REP_ERR_PACK;
			}
			* (uint32_t *) (val+vsize) = htonl(DATA_TYPE_EOF);
			vsize += sizeof(uint32_t);
			cache_setf(cd, val, vsize, PREFIX_FRIEND"%d", uin);
			free(val);
		} else {
			unpack_data("root", val, vsize, &q->replydata);
		}
	} else {
		dtc_err(fp, "get uin param failure");
		return REP_ERR_BADPARAM;
	}

	return REP_OK;
}

/*
 * input : uin(UINT) frienduin(UINT)
 * return: NORMAL REP_OK_ISFRIEND REP_OK_NOTFRIEND
 * reply : NULL
 */
static int uic_cmd_isfriend(struct queue_entry *q, struct cache *cd,
							fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	char key[64];
	int fuin, ret;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "frienduin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	fuin = c->v.ival;
	
	ret = uic_cmd_friendlist(q, cd, db, fp);
	if (PROCESS_OK(ret)) {
		sprintf(key, "%d", fuin);
        c = data_cell_search(q->replydata, false, DATA_TYPE_STRING, "friend");
        if (c != NULL) {
            c = data_cell_search(c, false, DATA_TYPE_U32, key);
            if (c != NULL) {
                data_cell_free(q->replydata);
                q->replydata = NULL;
                return REP_OK_ISFRIEND;
            }
        }
		data_cell_free(q->replydata);
		q->replydata = NULL;
		return REP_OK_NOTFRIEND;
	}
	return ret;
}

/*
 * input : uin(UINT) frienduin(UINT) groupid(UINT)
 * return: NORMAL REP_ERR_ALREADYFRIEND REP_ERR_ALREADYAPPLYED
 * reply : NULL
 */
static int uic_cmd_applyfriend(struct queue_entry *q, struct cache *cd,
							   fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	int uin, fuin, gid, ret;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "uin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	uin = c->v.ival;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "frienduin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	fuin = c->v.ival;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "groupid");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	gid = c->v.ival;

	ret = uic_cmd_isfriend(q, cd, db, fp);
	if (PROCESS_NOK(ret)) {
		dtc_err(fp, "judge friendship failure %d", ret);
		return ret;
	}
	
	if (ret == REP_OK_ISFRIEND) {
		dtc_warn(fp, "%d %d already friendship", uin, fuin);
		return REP_ERR_ALREADYFRIEND;
	}

	snprintf(db->sql, sizeof(db->sql), "SELECT * FROM user_friends WHERE "
			 " userid=%d AND friend_userid=%d AND beok=0;", uin, fuin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}
	if (fdb_fetch_row(db) == RET_DBOP_OK) {
		return REP_ERR_ALREADYAPPLYED;
	}
	
	snprintf(db->sql, sizeof(db->sql), "INSERT INTO user_friends "
			 " (userid, friend_userid, groupid, createtime, beok) "
			 " VALUES (%d, %d, %d, UNIX_TIMESTAMP(), 0)",
			 uin, fuin, gid);
	ret = fdb_exec(db);
	if (ret != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}
	
	return REP_OK;
}

/*
 * input : uin(UINT) frienduin(UINT)
 * return: NORMAL REP_ERR_ALREADYFRIEND REP_ERR_NOTAPPLY
 * reply : NULL
 */
static int uic_cmd_rejectapply(struct queue_entry *q, struct cache *cd,
							   fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	int uin, fuin, ret;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "uin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	uin = c->v.ival;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "frienduin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	fuin = c->v.ival;

	ret = uic_cmd_isfriend(q, cd, db, fp);
	if (PROCESS_NOK(ret)) {
		dtc_err(fp, "judge friendship failure %d", ret);
		return ret;
	}
	
	if (ret == REP_OK_ISFRIEND) {
		dtc_warn(fp, "%d %d already friendship", uin, fuin);
		return REP_ERR_ALREADYFRIEND;
	}

	snprintf(db->sql, sizeof(db->sql), "SELECT * FROM user_friends WHERE "
			 " userid=%d AND friend_userid=%d AND beok=0;", fuin, uin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}
	if (fdb_fetch_row(db) != RET_DBOP_OK) {
		return REP_ERR_NOTAPPLY;
	}

	snprintf(db->sql, sizeof(db->sql), "DELETE FROM user_friends WHERE "
			 " userid=%d AND friend_userid=%d AND beok=0;", fuin, uin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}

	return REP_OK;
}

/*
 * input : uin(UINT) frienduin(UINT) groupid(UINT)
 * return: NORMAL REP_OK_INSERT REP_OK_UPDATE REP_ERR_NOTAPPLY REP_ERR_ALREADYFRIEND
 * reply : NULL
 */
static int uic_cmd_confirmfriend(struct queue_entry *q, struct cache *cd,
								 fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	int uin, fuin, gid;
	int fgid;
	int ret, res;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "uin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	uin = c->v.ival;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "frienduin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	fuin = c->v.ival;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "groupid");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	gid = c->v.ival;

	ret = uic_cmd_isfriend(q, cd, db, fp);
	if (PROCESS_NOK(ret)) {
		dtc_err(fp, "judge friendship failure %d", ret);
		return ret;
	}
	
	if (ret == REP_OK_ISFRIEND) {
		dtc_warn(fp, "%d %d already friendship", uin, fuin);
		return REP_ERR_ALREADYFRIEND;
	}

	snprintf(db->sql, sizeof(db->sql), "SELECT groupid FROM user_friends "
			 " WHERE userid=%d AND friend_userid=%d AND beok=0;", fuin, uin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}
	if (fdb_fetch_row(db) != RET_DBOP_OK) {
		return REP_ERR_NOTAPPLY;
	} else {
		fgid = atoi(db->row[0]);
	}

	snprintf(db->sql, sizeof(db->sql), "UPDATE user_friends SET beok=1 "
			 " WHERE userid=%d;", fuin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}

	snprintf(db->sql, sizeof(db->sql), "SELECT * FROM user_friends "
			 " WHERE userid=%d AND friend_userid=%d AND beok=0;", uin, fuin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}
	if (fdb_fetch_row(db) == RET_DBOP_OK) {
		snprintf(db->sql, sizeof(db->sql), "UPDATE user_friends SET beok=1 "
				 " WHERE userid=%d AND friend_userid=%d;", uin, fuin);
		res = REP_OK_UPDATE;
	} else {
		snprintf(db->sql, sizeof(db->sql), "INSERT INTO user_friends "
				 " (userid, friend_userid, groupid, createtime, beok) "
				 " VALUES (%d, %d, %d, UNIX_TIMESTAMP(), 1);",
				 uin, fuin, gid);
		res = REP_OK_INSERT;
	}
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}
	
	snprintf(db->sql, sizeof(db->sql), "UPDATE user_friends_group SET "
			 " friendnum=friendnum+1 WHERE userid=%d AND fgroupid=%d", uin, gid);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
	}
	snprintf(db->sql, sizeof(db->sql), "UPDATE user_friends_group SET "
			 " friendnum=friendnum+1 WHERE userid=%d AND fgroupid=%d", fuin, fgid);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
	}
	
	snprintf(db->sql, sizeof(db->sql), "UPDATE home.user_info SET "
			 " friendnum=friendnum+1 WHERE userid=%d OR userid=%d", uin, fuin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
	}

	cache_delf(cd, PREFIX_FRIEND"%d", uin);
	cache_delf(cd, PREFIX_FRIEND"%d", fuin);
	
	return res;
}

/*
 * input : uin frienduin
 * return: NORMAL REP_ERR_NOTFRIEND
 * reply : NULL
 */
static int uic_cmd_delfriend(struct queue_entry *q, struct cache *cd,
							 fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	int uin, fuin, gid, fgid;
	char key[64];
	int ret;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "uin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	uin = c->v.ival;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "frienduin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	fuin = c->v.ival;

	ret = uic_cmd_friendlist(q, cd, db, fp);
	if (PROCESS_OK(ret)) {
		sprintf(key, "%d", fuin);
		c = data_cell_search(q->replydata, true, DATA_TYPE_U32, key);
		if (c != NULL) {
			gid = c->v.ival;
		} else {
			data_cell_free(q->replydata);
			q->replydata = NULL;
			dtc_warn(fp, "%d %d not friendship", uin, fuin);
			return REP_ERR_NOTFRIEND;
		}
		data_cell_free(q->replydata);
		q->replydata = NULL;
	} else {
		dtc_err(fp, "judge friendship failure %d", ret);
		return ret;
	}

	data_cell_add_u32(q->dataset, NULL, "uin", fuin);
    data_cell_add_u32(q->dataset, NULL, "frienduin", uin);
    
	ret = uic_cmd_friendlist(q, cd, db, fp);
	if (PROCESS_OK(ret)) {
		sprintf(key, "%d", uin);
		c = data_cell_search(q->replydata, true, DATA_TYPE_U32, key);
		if (c != NULL) {
			fgid = c->v.ival;
		} else {
			/* possible? */
			data_cell_free(q->replydata);
			q->replydata = NULL;
			dtc_warn(fp, "%d %d not friendship", uin, fuin);
			return REP_ERR_NOTFRIEND;
		}
		data_cell_free(q->replydata);
		q->replydata = NULL;
	} else {
		dtc_err(fp, "judge friendship failure %d", ret);
		return ret;
	}

#if 0
	/*
	 * recover uin for later use
     * later cache refresh is moved inside this func,
     * done by uin fuin, so, deprecated
	 */
	data_cell_add_u32(q->dataset, NULL, "uin", uin);
    data_cell_add_u32(q->dataset, NULL, "frienduin", fuin);
#endif
	
	snprintf(db->sql, sizeof(db->sql), "DELETE FROM user_friends WHERE "
			 " userid=%d AND friend_userid=%d;", uin, fuin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}
	snprintf(db->sql, sizeof(db->sql), "DELETE FROM user_friends WHERE "
			 " userid=%d AND friend_userid=%d;", fuin, uin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}
	
	snprintf(db->sql, sizeof(db->sql), "UPDATE user_friends_group SET "
			 " friendnum=friendnum-1 WHERE userid=%d AND fgroupid=%d;", uin, gid);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
	}
	snprintf(db->sql, sizeof(db->sql), "UPDATE user_friends_group SET "
			 " friendnum=friendnum-1 WHERE userid=%d AND fgroupid=%d;", fuin, fgid);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
	}

	snprintf(db->sql, sizeof(db->sql), "UPDATE home.user_info SET "
			 " friendnum=friendnum-1 WHERE userid=%d OR userid=%d", uin, fuin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
	}
	
	cache_delf(cd, PREFIX_FRIEND"%d", uin);
	cache_delf(cd, PREFIX_FRIEND"%d", fuin);
	
	return REP_OK;
}

/*
 * input : uin(UINT) maxnum(UINT) groupname(STRING)
 * return: NORMAL REP_ERR_MAXGROUPLIMIT REP_ERR_GROUPNAMEEXIST
 * reply : ["groupid": 22] OR ["groupid": 0]
 */
static int uic_cmd_addgroup(struct queue_entry *q, struct cache *cd,
							fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	int uin, max, ret;
	char *gname;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "uin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	uin = c->v.ival;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "maxnum");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	max = c->v.ival;

	c = data_cell_search(q->dataset, false, DATA_TYPE_STRING, "groupname");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	gname = fdb_escape_string(db, (char*)c->v.sval.val);
	if (gname == NULL) return REP_ERR_MEM;

	snprintf(db->sql, sizeof(db->sql), "SELECT COUNT(*) FROM user_friends_group "
			 " WHERE userid=%d;", uin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		ret = REP_ERR_DB;
		goto done;
	}
	if (fdb_fetch_row(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		ret = REP_ERR_DB;
		goto done;
	}
	if (atoi(db->row[0]) >= max) {
		dtc_warn(fp, "%d attemped to create more than %d groups", uin, max);
		ret = REP_ERR_MAXGROUPLIMIT;
		goto done;
	}

	snprintf(db->sql, sizeof(db->sql), "SELECT * FROM user_friends_group "
			 " WHERE userid=%d AND groupname='%s';", uin, gname);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		ret = REP_ERR_DB;
		goto done;
	}
	if (fdb_fetch_row(db) == RET_DBOP_OK) {
		ret = REP_ERR_GROUPNAMEEXIST;
		goto done;
	}

	snprintf(db->sql, sizeof(db->sql), "INSERT INTO user_friends_group "
			 " (userid, groupname) VALUES (%d, '%s');", uin, gname);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		ret = REP_ERR_DB;
		goto done;
	}

	reply_add_u32(q, NULL, "groupid", (uint32_t)fdb_get_last_id(db));

	ret = REP_OK;
	
 done:
	free(gname);
	return ret;
}

/*
 * input : uin(UINT) groupid(UINT) groupname(STRING)
 * return: NORMAL REP_ERR_GROUPNAMEEXIST REP_ERR_NOTOWNER
 * reply : ["groupid": 22] OR ["groupid": 0]
 */
static int uic_cmd_modgroup(struct queue_entry *q, struct cache *cd,
							fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	int uin, gid, ret;
	char *gname;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "uin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	uin = c->v.ival;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "groupid");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	gid = c->v.ival;

	if (gid == 1)
		return REP_ERR_GROUPREADONLY;

	c = data_cell_search(q->dataset, false, DATA_TYPE_STRING, "groupname");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	gname = fdb_escape_string(db, (char*)c->v.sval.val);
	if (gname == NULL) return REP_ERR_MEM;

	snprintf(db->sql, sizeof(db->sql), "SELECT * FROM user_friends_group "
			 " WHERE userid=%d AND groupname='%s';", uin, gname);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		ret = REP_ERR_DB;
		goto done;
	}
	if (fdb_fetch_row(db) == RET_DBOP_OK) {
		ret = REP_ERR_GROUPNAMEEXIST;
		goto done;
	}

	snprintf(db->sql, sizeof(db->sql), "SELECT userid FROM user_friends_group "
			 " WHERE fgroupid=%d;", gid);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		ret = REP_ERR_DB;
		goto done;
	}
	if (fdb_fetch_row(db) != RET_DBOP_OK || atoi(db->row[0]) != uin) {
		ret = REP_ERR_NOTOWNER;
		goto done;
	}

	snprintf(db->sql, sizeof(db->sql), "UPDATE user_friends_group SET "
			 "groupname='%s' WHERE fgroupid=%d;", gname, gid);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		ret = REP_ERR_DB;
		goto done;
	}
	
	reply_add_u32(q, NULL, "groupid", gid);

	ret = REP_OK;
	
 done:
	free(gname);
	return ret;
}

/*
 * input : uin(UINT) groupid(UINT)
 * return: NORMAL REP_ERR_NOTOWNER
 * reply : NULL
 */
static int uic_cmd_delgroup(struct queue_entry *q, struct cache *cd,
							fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	int uin, gid;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "uin");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	uin = c->v.ival;

	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "groupid");
	if (c == NULL)
		return REP_ERR_BADPARAM;
	gid = c->v.ival;

	if (gid == 1)
		return REP_ERR_GROUPREADONLY;

	snprintf(db->sql, sizeof(db->sql), "SELECT userid FROM user_friends_group "
			 " WHERE fgroupid=%d;", gid);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}
	if (fdb_fetch_row(db) != RET_DBOP_OK || atoi(db->row[0]) != uin) {
		return REP_ERR_NOTOWNER;
	}

	snprintf(db->sql, sizeof(db->sql), "DELETE FROM user_friends_group "
			 " WHERE fgroupid=%d;", gid);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}

	snprintf(db->sql, sizeof(db->sql), "UPDATE user_friends SET groupid=%d WHERE "
			 " userid=%d AND groupid=%d;", DEFAULT_GROUP_ID, uin, gid);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}

	cache_delf(cd, PREFIX_FRIEND"%d", uin);

	return REP_OK;
}

/*
 * input : uin(UINT)
 * return: NORMAL
 * reply : ["incept": ["1": 1, "2": 2...], "msgset": 1/2/3] OR ["incept": []]
 */
static int uic_cmd_mysetting(struct queue_entry *q, struct cache *cd,
							 fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	unsigned char *val = NULL;
	size_t vsize = 0;
	int uin, hit, ret;
	
	c = data_cell_search(q->dataset, false, DATA_TYPE_U32, "uin");
	if (c != NULL) {
		uin = c->v.ival;
		reply_add_array(q, NULL, "incept");
		hit = cache_getf(cd, &val, &vsize, PREFIX_SETTING"%d", uin);
		if (hit == 0) {
			snprintf(db->sql, sizeof(db->sql), "SELECT incept_dynamic, receive_msg "
					 " FROM home.user_info WHERE userid=%d;", uin);
			ret = fdb_exec(db);
			if (ret != RET_DBOP_OK) {
				dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
				return REP_ERR_DB;
			}
			if (fdb_fetch_row(db) == RET_DBOP_OK) {
				if (strcmp(db->row[0], "")) {
					char *ids[MAX_INCEPT_NUM];
					size_t num = explode(',', db->row[0], ids, MAX_INCEPT_NUM-1);
					int loopi = 0;
					for (loopi = 0; loopi <= num; loopi++) {
						reply_add_u32(q, "incept", ids[loopi], atoi(ids[loopi]));
					}
				}
				reply_add_u32(q, NULL, "msgset", atoi(db->row[1]));
			}
			val = calloc(1, MAX_PACKET_LEN);
			if (val == NULL) return REP_ERR_MEM;
			vsize = pack_data_array(NULL, q->replydata, val,
									MAX_PACKET_LEN - RESERVE_SIZE);
			if (vsize == 0) {
				free(val);
				return REP_ERR_PACK;
			}
			* (uint32_t *) (val+vsize) = htonl(DATA_TYPE_EOF);
			vsize += sizeof(uint32_t);
			cache_setf(cd, val, vsize, PREFIX_SETTING"%d", uin);
		}
		q->req->reply_long(q->req, REP_OK, val, vsize);
		if (hit == 0) free(val);
	} else {
		dtc_err(fp, "get uin param failure");
		return REP_ERR_BADPARAM;
	}

	return REP_OK;
}

static void uic_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct uic_entry *e = (struct uic_entry*)entry;
	int ret = REP_OK;
	
	FILE *fp = e->logf;
	fdb_t *db = e->db;
	struct cache *cd = e->cd;
	struct uic_stats *st = &(e->st);

	st->msg_total++;

	dtc_dbg(fp, "process cmd %u", q->operation);
	switch (q->operation) {
	case REQ_CMD_FRIENDLIST:
		ret = uic_cmd_friendlist(q, cd, db, fp);
		break;
	case REQ_CMD_ISFRIEND:
		ret = uic_cmd_isfriend(q, cd, db, fp);
		break;
	case REQ_CMD_APPLYFRIEND:
		ret = uic_cmd_applyfriend(q, cd, db, fp);
		break;
	case REQ_CMD_REJECTAPPLY:
		ret = uic_cmd_rejectapply(q, cd, db, fp);
		break;
	case REQ_CMD_CONFIRMFRIEND:
		ret = uic_cmd_confirmfriend(q, cd, db, fp);
		break;
	case REQ_CMD_DELFRIEND:
		ret = uic_cmd_delfriend(q, cd, db, fp);
		break;
	case REQ_CMD_ADDGROUP:
		ret = uic_cmd_addgroup(q, cd, db, fp);
		break;
	case REQ_CMD_MODGROUP:
		ret = uic_cmd_modgroup(q, cd, db, fp);
		break;
	case REQ_CMD_DELGROUP:
		ret = uic_cmd_delgroup(q, cd, db, fp);
		break;
	case REQ_CMD_MYSETTING:
		ret = uic_cmd_mysetting(q, cd, db, fp);
		break;
	case REQ_CMD_STATS:
		st->msg_stats++;
		ret = REP_OK;
		reply_add_ulong(q, NULL, "msg_total", st->msg_total);
		reply_add_ulong(q, NULL, "msg_unrec", st->msg_unrec);
		reply_add_ulong(q, NULL, "msg_badparam", st->msg_badparam);
		reply_add_ulong(q, NULL, "msg_stats", st->msg_stats);
		reply_add_ulong(q, NULL, "proc_suc", st->proc_suc);
		reply_add_ulong(q, NULL, "proc_fai", st->proc_fai);
	default:
		st->msg_unrec++;
		ret = REP_ERR_UNKREQ;
		break;
	}
	if (PROCESS_OK(ret)) {
		st->proc_suc++;
	} else {
		st->proc_fai++;
		dtc_err(fp, "process %u failed %d\n", q->operation, ret);
	}
	if (q->req->flags & FLAGS_SYNC) {
			reply_trigger(q, ret);
	}
}

static void uic_stop_driver(struct event_entry *entry)
{
	struct uic_entry *e = (struct uic_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	dtc_leave(e->logf);
	fdb_free(&e->db);
	cache_free(e->cd);
}



static struct event_entry* uic_init_driver(void)
{
	struct uic_entry *e = calloc(1, sizeof(struct uic_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = uic_process_driver;
	e->base.stop_driver = uic_stop_driver;

	e->logf = dtc_init(hdf_get_value(g_cfg, CONFIG_PATH".logfile",
									 TC_ROOT"plugin/uic"));
	if (e->logf == NULL) {
		wlog("open log file failure\n");
		goto error;
	}
	
	if (fdb_init_long(&e->db, hdf_get_value(g_cfg, CONFIG_PATH".ip", "127.0.0.1"),
					  hdf_get_value(g_cfg, CONFIG_PATH".user", "test"),
					  hdf_get_value(g_cfg, CONFIG_PATH".pass", "test"),
					  hdf_get_value(g_cfg, CONFIG_PATH".name", "user_info"),
					  (unsigned int)hdf_get_int_value(g_cfg, CONFIG_PATH".port", 0))
		!= RET_DBOP_OK) {
		wlog("init %s failure %s\n", PLUGIN_NAME, fdb_error(e->db));
		goto error;
	}

	e->cd = cache_create(hdf_get_int_value(g_cfg, CONFIG_PATH".numobjs", 1024), 0);
	if (e->cd == NULL) {
		wlog("init cache failure");
		goto error;
	}
	
	return (struct event_entry*)e;
	
 error:
	if (e->base.name) free(e->base.name);
	if (e->logf) dtc_leave(e->logf);
	if (e->db) fdb_free(&e->db);
	if (e->cd) cache_free(e->cd);
	free(e);
	return NULL;
}

struct event_driver uic_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = uic_init_driver,
};
