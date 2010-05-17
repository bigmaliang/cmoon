#include "mevent_plugin.h"
#include "mevent_dyn.h"

#define PLUGIN_NAME	"dyn"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct dyn_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct dyn_entry {
	struct event_entry base;
	mdb_conn *db;
	struct cache *cd;
	struct dyn_stats st;
};

/*
 * input : uname(STR) aname(STR)
 * return: NORMAL
 * reply : ["oname": "aaa", ...] OR []
 */
static int dyn_cmd_joinget(struct queue_entry *q, struct cache *cd,
						   mdb_conn *db)
{
	unsigned char *val = NULL;
	size_t vsize = 0;
	int hit, ret;
	
	char *uname, *aname, *oname, *ip, *refer, *url, *title, *tmp;
	int id, uid, aid, oid, retcode, cnt = 0;

	REQ_GET_PARAM_STR(q->hdfrcv, "uname", uname);
	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	uid = hash_string(uname);
	aid = hash_string(aname);

	hit = cache_getf(cd, &val, &vsize, PREFIX_DYN"%d%d", uid, aid);
	if (hit == 0) {
		ret = mdb_exec(db, NULL, "SELECT id, uid, uname, aid, aname, "
					   " oid, oname, ip, refer, url, title, retcode FROM "
					   " lcsjoin WHERE uid=%d AND aid=%d ORDER BY id DESC;",
					   NULL, uid, aid);
		if (ret != MDB_ERR_NONE) {
			mtc_err("exec failure %s", mdb_get_errmsg(db));
			return REP_ERR_DB;
		}
		while (mdb_get(db, "iisisisssssi", &id, &uid, &tmp, &aid, &tmp,
					   &oid, &oname, &ip, &refer, &url, &title, &retcode)
			   == MDB_ERR_NONE) {
			hdf_set_valuef(q->hdfsnd, "%d.id=%d", cnt, id);
			hdf_set_valuef(q->hdfsnd, "%d.uid=%d", cnt, uid);
			hdf_set_valuef(q->hdfsnd, "%d.aid=%d", cnt, aid);
			hdf_set_valuef(q->hdfsnd, "%d.oid=%d", cnt, oid);
			hdf_set_valuef(q->hdfsnd, "%d.retcode=%d", cnt, retcode);

			hdf_set_valuef(q->hdfsnd, "%d.uname=%s", cnt, uname);
			hdf_set_valuef(q->hdfsnd, "%d.aname=%s", cnt, aname);
			hdf_set_valuef(q->hdfsnd, "%d.oname=%s", cnt, oname);
			hdf_set_valuef(q->hdfsnd, "%d.ip=%s", cnt, ip);
			hdf_set_valuef(q->hdfsnd, "%d.refer=%s", cnt, refer);
			hdf_set_valuef(q->hdfsnd, "%d.url=%s", cnt, url);
			hdf_set_valuef(q->hdfsnd, "%d.title=%s", cnt, title);
			cnt++;
		}
		val = calloc(1, MAX_PACKET_LEN);
		if (val == NULL) {
			return REP_ERR_MEM;
		}
		vsize = pack_hdf(q->hdfsnd, val);
		cache_setf(cd, val, vsize, PREFIX_DYN"%d%d", uid, aid);
		free(val);
	} else {
		unpack_hdf(val, vsize, &q->hdfsnd);
	}
	
	return REP_OK;
}

/*
 * input : aname(STR) uname(STR) [...]
 * return: NORMAL
 * reply : NULL
 */
static int dyn_cmd_joinset(struct queue_entry *q, struct cache *cd,
						  mdb_conn *db)
{
	char *uname, *aname, *oname, *ip, *refer, *url, *title;
	int id, uid, aid, retcode = 0, ret;

	REQ_GET_PARAM_STR(q->hdfrcv, "uname", uname);
	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	uid = hash_string(uname);
	aid = hash_string(aname);

	REQ_FETCH_PARAM_STR(q->hdfrcv, "oname", oname);
	REQ_FETCH_PARAM_STR(q->hdfrcv, "ip", ip);
	REQ_FETCH_PARAM_STR(q->hdfrcv, "refer", refer);
	REQ_FETCH_PARAM_STR(q->hdfrcv, "url", url);
	REQ_FETCH_PARAM_STR(q->hdfrcv, "title", title);
	REQ_FETCH_PARAM_INT(q->hdfrcv, "retcode", retcode);
	oname = oname ? oname: "";
	ip = ip ? ip: "";
	refer = refer ? refer: "";
	url = url ? url: "";
	title = title ? title: "";

	ret = mdb_exec(db, NULL, "INSERT INTO lcsjoin (uid, uname, "
				   " aid, aname, oid, oname, ip, refer, url, title, retcode) "
				   " VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11) RETURNING id;",
				   "isisisssssi", uid, uname, aid, aname,
				   hash_string(oname), oname, ip, refer, url, title, retcode);
	if (ret != MDB_ERR_NONE) {
		mtc_err("exec failure %s", mdb_get_errmsg(db));
		return REP_ERR_DB;
	}
	
	if (mdb_get(db, "i", &id) == MDB_ERR_NONE) {
		hdf_set_int_value(q->hdfsnd, "id", id);
	}
	
	cache_delf(cd, PREFIX_DYN"%d%d", uid, aid);

	return REP_OK;
}

static void dyn_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct dyn_entry *e = (struct dyn_entry*)entry;
	int ret = REP_OK;
	
	mdb_conn *db = e->db;
	struct cache *cd = e->cd;
	struct dyn_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, ret);
	case REQ_CMD_JOINGET:
		ret = dyn_cmd_joinget(q, cd, db);
		break;
	case REQ_CMD_JOINSET:
		ret = dyn_cmd_joinset(q, cd, db);
		break;
	case REQ_CMD_STATS:
		st->msg_stats++;
		ret = REP_OK;
		hdf_set_int_value(q->hdfsnd, "msg_total", st->msg_total);
		hdf_set_int_value(q->hdfsnd, "msg_unrec", st->msg_unrec);
		hdf_set_int_value(q->hdfsnd, "msg_badparam", st->msg_badparam);
		hdf_set_int_value(q->hdfsnd, "msg_stats", st->msg_stats);
		hdf_set_int_value(q->hdfsnd, "proc_suc", st->proc_suc);
		hdf_set_int_value(q->hdfsnd, "proc_fai", st->proc_fai);
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
		mtc_err("process %u failed %d", q->operation, ret);
	}
	if (q->req->flags & FLAGS_SYNC) {
		reply_trigger(q, ret);
	}
}

static void dyn_stop_driver(struct event_entry *entry)
{
	struct dyn_entry *e = (struct dyn_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	mdb_destroy(e->db);
	cache_free(e->cd);
}



static struct event_entry* dyn_init_driver(void)
{
	struct dyn_entry *e = calloc(1, sizeof(struct dyn_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = dyn_process_driver;
	e->base.stop_driver = dyn_stop_driver;

	char *dbsn = hdf_get_value(g_cfg, CONFIG_PATH".dbsn", NULL);
	if (mdb_init(&e->db, dbsn) != RET_RBTOP_OK) {
		wlog("init %s failure %s\n", dbsn, mdb_get_errmsg(e->db));
		goto error;
	} else {
		mtc_info("init %s ok", dbsn);
	}
	
	e->cd = cache_create(hdf_get_int_value(g_cfg, CONFIG_PATH".numobjs", 1024), 0);
	if (e->cd == NULL) {
		wlog("init cache failure");
		goto error;
	}
	
	return (struct event_entry*)e;
	
error:
	if (e->base.name) free(e->base.name);
	if (e->db) mdb_destroy(e->db);
	if (e->cd) cache_free(e->cd);
	free(e);
	return NULL;
}

struct event_driver dyn_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = dyn_init_driver,
};
