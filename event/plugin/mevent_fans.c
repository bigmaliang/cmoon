#include "mevent_plugin.h"
#include "mevent_fans.h"

#define PLUGIN_NAME	"fans"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct fans_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct fans_entry {
	struct event_entry base;
	FILE *logf;
	fdb_t *db;
	struct cache *cd;
	struct fans_stats st;
};

/*
 * input : uin(UINT)
 * return: NORMAL
 * reply : ["fans": [ "100": 1269485819, "3": 1269485824, ...]] OR ["fans":[]]
 */
static int fans_cmd_fanslist(struct queue_entry *q, struct cache *cd,
							 fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	unsigned char *val = NULL;
	size_t vsize = 0;
	int uin, hit, count = 0, ret;

	REQ_GET_PARAM_U32(c, q, false, "uin", uin);
	
	reply_add_array(q, NULL, "fans");
	hit = cache_getf(cd, &val, &vsize, PREFIX_FANS"%d", uin);
	if (hit == 0) {
		snprintf(db->sql, sizeof(db->sql), "SELECT userid, attention_time FROM "
				 " user_attention WHERE attention_userid=%d ORDER BY uaid DESC;", uin);
		ret = fdb_exec(db);
		if (ret != RET_DBOP_OK) {
			dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
			return REP_ERR_DB;
		}
		while (count < MAX_FANS_NUM && (fdb_fetch_row(db) == RET_DBOP_OK)) {
			reply_add_u32(q, "fans", db->row[0], atoi(db->row[1]));
			count++;
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
		cache_setf(cd, val, vsize, PREFIX_FANS"%d", uin);
		free(val);
	} else {
		unpack_data("root", val, vsize, &q->replydata);
	}

	return REP_OK;
}

/*
 * input : uin(UINT)
 * return: NORMAL
 * reply : ["idols": [ "100": 1269485819, "3": 1269485824, ...]] OR ["idols":[]]
 */
static int fans_cmd_idollist(struct queue_entry *q, struct cache *cd,
							 fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	unsigned char *val = NULL;
	size_t vsize = 0;
	int uin, hit, ret;

	REQ_GET_PARAM_U32(c, q, false, "uin", uin);
	
	reply_add_array(q, NULL, "idols");
	hit = cache_getf(cd, &val, &vsize, PREFIX_IDOLS"%d", uin);
	if (hit == 0) {
		snprintf(db->sql, sizeof(db->sql), "SELECT attention_userid, attention_time "
				 " FROM user_attention WHERE userid=%d ORDER BY uaid DESC;", uin);
		ret = fdb_exec(db);
		if (ret != RET_DBOP_OK) {
			dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
			return REP_ERR_DB;
		}
		while (fdb_fetch_row(db) == RET_DBOP_OK) {
			reply_add_u32(q, "idols", db->row[0], atoi(db->row[1]));
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
		cache_setf(cd, val, vsize, PREFIX_IDOLS"%d", uin);
		free(val);
	} else {
		unpack_data("root", val, vsize, &q->replydata);
	}

	return REP_OK;
}

/*
 * input : uin(UINT) otheruin(UINT)
 * return: NORMAL REP_OK_CLEAN REP_OK_ISFANS REP_OK_ISIDOL REP_OK_ISFANSEO
 * reply : ["attention_time": 1269485819]] OR ["attention_time":[]]
 */
static int fans_cmd_followtype(struct queue_entry *q, struct cache *cd,
							   fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	char key[64];
	bool isfans, isidol;
	int ouin, time = 0, ret;

	REQ_GET_PARAM_U32(c, q, false, "otheruin", ouin);
	
	isfans = isidol = false;
	
	ret = fans_cmd_fanslist(q, cd, db, fp);
	if (PROCESS_OK(ret)) {
		c = data_cell_search(q->replydata, false, DATA_TYPE_ARRAY, "fans");
		if (c != NULL) {
			sprintf(key, "%d", ouin);
			c = data_cell_search(c, false, DATA_TYPE_U32, key);
			if (c != NULL) {
				time = c->v.ival;
				data_cell_free(q->replydata);
				q->replydata = NULL;
				/* uin's fans list has otheruin, so, uin is otheruin's idol */
				isidol = true;
			}
		}
		data_cell_free(q->replydata);
		q->replydata = NULL;
	} else {
		return ret;
	}

	ret = fans_cmd_idollist(q, cd, db, fp);
	if (PROCESS_OK(ret)) {
		c = data_cell_search(q->replydata, false, DATA_TYPE_ARRAY, "idols");
		if (c != NULL) {
			sprintf(key, "%d", ouin);
			c = data_cell_search(c, false, DATA_TYPE_U32, key);
			if (c != NULL) {
				if (time < c->v.ival)
					time = c->v.ival;
				data_cell_free(q->replydata);
				q->replydata = NULL;
				/* uin's idol list has otheruin, so, uin is otheruin's fans */
				isfans = true;
			}
		}
		data_cell_free(q->replydata);
		q->replydata = NULL;
	}
	
	if (isfans || isidol)
		reply_add_u32(q, NULL, "attention_time", time);

	if (isfans && isidol) {
		return REP_OK_ISFANSEO;
	} else if (isfans) {
		return REP_OK_ISFANS;
	} else if (isidol) {
		return REP_OK_ISIDOL;
	}
	
	return REP_OK_CLEAN;
}

/*
 * input : uin(UINT) otheruin(UINT)
 * return: NORMAL REP_OK_ISFANSEO REP_ERR_ALREADYFANS
 * reply : NULL
 */
static int fans_cmd_follow(struct queue_entry *q, struct cache *cd,
						   fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	int uin, ouin, ret;

	REQ_GET_PARAM_U32(c, q, false, "uin", uin);
	REQ_GET_PARAM_U32(c, q, false, "otheruin", ouin);

	ret = fans_cmd_followtype(q, cd, db, fp);
	if (PROCESS_NOK(ret)) {
		dtc_err(fp, "judge fansship failure %d", ret);
		return ret;
	}
	if (ret == REP_OK_ISFANS || ret == REP_OK_ISFANSEO) {
		dtc_warn(fp, "%d alread followed %d", uin, ouin);
		return REP_ERR_ALREADYFANS;
	}

	snprintf(db->sql, sizeof(db->sql), "INSERT INTO user_attention "
			 " (userid, attention_userid, attention_time) "
			 " VALUES (%d, %d, UNIX_TIMESTAMP());", uin, ouin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}

	cache_delf(cd, PREFIX_FANS"%d", ouin);
	cache_delf(cd, PREFIX_IDOLS"%d", uin);

	if (ret == REP_OK_ISIDOL)
		return REP_OK_ISFANSEO;
	return REP_OK;
}

/*
 * input : uin(UINT) otheruin(UINT)
 * return: NORMAL REP_OK_ISIDOL REP_ERR_NOTFANS
 * reply : NULL
 */
static int fans_cmd_unfollow(struct queue_entry *q, struct cache *cd,
							 fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	int uin, ouin, ret;

	REQ_GET_PARAM_U32(c, q, false, "uin", uin);
	REQ_GET_PARAM_U32(c, q, false, "otheruin", ouin);

	ret = fans_cmd_followtype(q, cd, db, fp);
	if (PROCESS_NOK(ret)) {
		dtc_err(fp, "judge fansship failure %d", ret);
		return ret;
	}
	if (ret != REP_OK_ISFANS && ret != REP_OK_ISFANSEO) {
		dtc_warn(fp, "%d not %d's fans", uin, ouin);
		return REP_ERR_NOTFANS;
	}

	snprintf(db->sql, sizeof(db->sql), "DELETE FROM user_attention "
			 " WHERE userid=%d AND attention_userid=%d;", uin, ouin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}

	cache_delf(cd, PREFIX_FANS"%d", ouin);
	cache_delf(cd, PREFIX_IDOLS"%d", uin);

	if (ret == REP_OK_ISIDOL)
		return REP_OK_ISIDOL;
	return REP_OK;
}

static void fans_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct fans_entry *e = (struct fans_entry*)entry;
	int ret = REP_OK;
	
	FILE *fp = e->logf;
	fdb_t *db = e->db;
	struct cache *cd = e->cd;
	struct fans_stats *st = &(e->st);

	st->msg_total++;
	
	dtc_dbg(fp, "process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, db, fp, ret);
	case REQ_CMD_FANSLIST:
		ret = fans_cmd_fanslist(q, cd, db, fp);
		break;
	case REQ_CMD_IDOLLIST:
		ret = fans_cmd_idollist(q, cd, db, fp);
		break;
	case REQ_CMD_FOLLOWTYPE:
		ret = fans_cmd_followtype(q, cd, db, fp);
		break;
	case REQ_CMD_FOLLOW:
		ret = fans_cmd_follow(q, cd, db, fp);
		break;
	case REQ_CMD_UNFOLLOW:
		ret = fans_cmd_unfollow(q, cd, db, fp);
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
		break;
	default:
		st->msg_unrec++;
		ret = REP_ERR_UNKREQ;
		break;
	}
	if (PROCESS_OK(ret)) {
		st->proc_suc++;
	} else {
		st->proc_fai++;
        if (ret == REP_ERR_BADPARAM) {
            st->msg_badparam++;
        }
		dtc_err(fp, "process %u failed %d", q->operation, ret);
	}
	if (q->req->flags & FLAGS_SYNC) {
		reply_trigger(q, ret);
	}
}

static void fans_stop_driver(struct event_entry *entry)
{
	struct fans_entry *e = (struct fans_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	dtc_leave(e->logf);
	fdb_free(&e->db);
	cache_free(e->cd);
}



static struct event_entry* fans_init_driver(void)
{
	struct fans_entry *e = calloc(1, sizeof(struct fans_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = fans_process_driver;
	e->base.stop_driver = fans_stop_driver;

	e->logf = dtc_init(hdf_get_value(g_cfg, CONFIG_PATH".logfile",
									 TC_ROOT"plugin/fans"));
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

struct event_driver fans_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = fans_init_driver,
};
