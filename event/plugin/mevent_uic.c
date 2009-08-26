#include "mevent_plugin.h"

#define PLUGIN_NAME	"uic"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

enum {
	REQ_CMD_FRIENDLIST = 1001,
	REQ_CMD_ISFRIEND,
	REQ_CMD_ADDFRIEND,
	REQ_CMD_DELFRIEND,
	REQ_CMD_MYSETTING
} req_cmd;

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

#define RET_DBOP_BADPARAM	2049
#define LEN_CACHE_KEY		1024
#define MAX_INCEPT_NUM		100

static void pack_userfriend(struct data_cell *c, unsigned char **val, size_t *vlen)
{
	struct data_cell *pc, *cc;
	unsigned char *buf;
	size_t len = 0, pos = 0;

	*val = NULL;
	*vlen = 0;

	pc = data_cell_search(c, false, DATA_TYPE_ARRAY, "friend");
	if (pc == NULL) return;

	data_cell_array_iterate(pc, cc) {
		len += cc->v.sval.vsize;
	}

	buf = malloc(len);
	if (buf == NULL) return;
	
	data_cell_array_iterate(pc, cc) {
		memcpy(buf+pos, cc->v.sval.val, cc->v.sval.vsize);
		pos += cc->v.sval.vsize;
	}

	*val = buf;
	*vlen = len;
}

static void unpack_userfriend(struct data_cell *c, unsigned char *val, size_t vlen)
{
	unsigned char *p = val;
	size_t plen = 0;

	while (*p != '\0' && plen < vlen) {
		data_cell_add_str(c, "friend", (char*)p, (char*)p);
		while (*p != '\0') {
			p++;
			plen++;
		}
		plen++;
		p++;
	}
}

static int uic_cmd_friendlist(struct queue_entry *q, struct cache *cd,
							  fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	char key[LEN_CACHE_KEY];
	unsigned char *val = NULL;
	size_t ksize, vsize = 0;
	int ret, hit;
	
	c = data_cell_search(q->dataset, false, DATA_TYPE_STRING, "uin");
	if (c != NULL) {
		reply_add_array(q, NULL, "friend");
		ksize = snprintf(key, sizeof(key), "Friend%s", c->v.sval.val);
		hit = cache_get(cd, (unsigned char*)key, ksize, &val, &vsize);
		if (hit == 0) {
			snprintf(db->sql, sizeof(db->sql), "SELECT friend_userid FROM "
					 " user_friends WHERE userid=%s;", c->v.sval.val);
			ret = fdb_exec(db);
			if (ret != RET_DBOP_OK) {
				dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
				return ret;
			}
			while (fdb_fetch_row(db) == RET_DBOP_OK) {
				reply_add_str(q, "friend", db->row[0], db->row[0]);
			}
			pack_userfriend(q->replydata, &val, &vsize);
			cache_set(cd, (unsigned char*)key, ksize, val, vsize);
			free(val);
		} else {
			unpack_userfriend(q->replydata, val, vsize);
		}
		reply_trigger(q, REP_OK);
	} else {
		dtc_err(fp, "get uin param failure");
		return RET_DBOP_BADPARAM;
	}

	return RET_DBOP_OK;
}

static void pack_usersetting(struct data_cell *c, unsigned char **val, size_t *vlen)
{
	struct data_cell *pc, *cc;
	unsigned char *buf;
	size_t len = 0, pos = 0;

	*val = NULL;
	*vlen = 0;

	len = sizeof(int);
	
	pc = data_cell_search(c, false, DATA_TYPE_ARRAY, "incept");
	if (pc != NULL) {
		data_cell_array_iterate(pc, cc) {
			len += cc->v.sval.vsize;
		}
	}

	pc = data_cell_search(c, false, DATA_TYPE_STRING, "msgset");
	if (pc != NULL)
		len += pc->v.sval.vsize;
	
	buf = malloc(len);
	if (buf == NULL) return;

	pc = data_cell_search(c, false, DATA_TYPE_ARRAY, "incept");
	*(int*)(buf+pos) = data_cell_length(pc);
	pos += sizeof(int);
	if (pc != NULL) {
		data_cell_array_iterate(pc, cc) {
			memcpy(buf+pos, cc->v.sval.val, cc->v.sval.vsize);
			pos += cc->v.sval.vsize;
		}
	}

	pc = data_cell_search(c, false, DATA_TYPE_STRING, "msgset");
	if (pc != NULL) {
		memcpy(buf+pos, pc->v.sval.val, pc->v.sval.vsize);
		pos += pc->v.sval.vsize;
	}

	*val = buf;
	*vlen = len;
}

static void unpack_usersetting(struct data_cell *c, unsigned char *val, size_t vlen)
{
	unsigned char *p = val;
	size_t plen = 0;
	int num, i;

	num = *(int*)val;
	p += sizeof(int);
	plen += sizeof(int);

	data_cell_add_array(c, NULL, "incept");
	
	for (i = 0; i < num && plen < vlen; i++) {
		data_cell_add_str(c, "incept", (char*)p, (char*)p);
		while (*p != '\0') {
			p++;
			plen++;
		}
		p++;
		plen++;
	}

	if (*p != '\0' && plen < vlen) {
		data_cell_add_str(c, "msgset", (char*)p, (char*)p);
	}
}

static int uic_cmd_mysetting(struct queue_entry *q, struct cache *cd,
							 fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	char key[LEN_CACHE_KEY];
	unsigned char *val = NULL;
	size_t ksize, vsize = 0;
	int ret, hit;
	
	c = data_cell_search(q->dataset, false, DATA_TYPE_STRING, "uin");
	if (c != NULL) {
		ksize = snprintf(key, sizeof(key), "Setting%s", c->v.sval.val);
		hit = cache_get(cd, (unsigned char*)key, ksize, &val, &vsize);
		if (hit == 0) {
			snprintf(db->sql, sizeof(db->sql), "SELECT incept_dynamic, receive_msg "
					 " FROM user_info WHERE userid=%s;", c->v.sval.val);
			ret = fdb_exec(db);
			if (ret != RET_DBOP_OK) {
				dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
				return ret;
			}
			if (fdb_fetch_row(db) == RET_DBOP_OK) {
				if (strcmp(db->row[0], "")) {
					reply_add_array(q, NULL, "incept");
					char *ids[MAX_INCEPT_NUM];
					size_t num = explode(',', db->row[0], ids, MAX_INCEPT_NUM-1);
					int loopi = 0;
					for (loopi = 0; loopi <= num; loopi++) {
						reply_add_str(q, "incept", ids[loopi], ids[loopi]);
					}
				}
				reply_add_str(q, NULL, "msgset", db->row[1]);
			}
			pack_usersetting(q->replydata, &val, &vsize);
			cache_set(cd, (unsigned char*)key, ksize, val, vsize);
			free(val);
		} else {
			unpack_usersetting(q->replydata, val, vsize);
		}
		reply_trigger(q, REP_OK);
	} else {
		dtc_err(fp, "get uin param failure");
		return RET_DBOP_BADPARAM;
	}

	return RET_DBOP_OK;
}

static void uic_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct uic_entry *e = (struct uic_entry*)entry;
	int ret = RET_DBOP_OK;
	
	e = (struct uic_entry*)entry;
	FILE *fp = e->logf;
	fdb_t *db = e->db;
	struct cache *cd = e->cd;
	struct uic_stats *st = &(e->st);

	st->msg_total++;

	if (q->operation == REQ_CMD_FRIENDLIST) {
		ret = uic_cmd_friendlist(q, cd, db, fp);
		if (ret != RET_DBOP_OK) {
			st->proc_fai++;
			dtc_err(fp, "process failed %d\n", ret);
			q->req->reply_err(q->req, ret);
		}
	} else if (q->operation == REQ_CMD_MYSETTING) {
		ret = uic_cmd_mysetting(q, cd, db, fp);
		if (ret != RET_DBOP_OK) {
			st->proc_fai++;
			dtc_err(fp, "process failed %d\n", ret);
			q->req->reply_err(q->req, ret);
		}
	} else if (q->operation == REQ_CMD_STATS) {
		st->msg_stats++;
		if ((q->req->flags & FLAGS_SYNC)) {
			reply_add_ulong(q, NULL, "msg_total", st->msg_total);
			reply_add_ulong(q, NULL, "msg_unrec", st->msg_unrec);
			reply_add_ulong(q, NULL, "msg_badparam", st->msg_badparam);
			reply_add_ulong(q, NULL, "msg_stats", st->msg_stats);
			reply_add_ulong(q, NULL, "proc_suc", st->proc_suc);
			reply_add_ulong(q, NULL, "proc_fai", st->proc_fai);
			reply_trigger(q, REP_OK);
			return;
		}
	} else {
		st->msg_unrec++;
		ret = ERR_UNKREQ;
	}
	
	if ((q->req->flags & FLAGS_SYNC)) {
		if (ret == RET_DBOP_OK) {
			q->req->reply_mini(q->req, REP_OK);
		} else {
			q->req->reply_err(q->req, ret);
		}
		return;
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
