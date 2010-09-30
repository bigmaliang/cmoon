#include "mevent_plugin.h"
#include "mevent_aic.h"

#define PLUGIN_NAME	"aic"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct aic_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct aic_entry {
	struct event_entry base;
	mdb_conn *db;
	struct cache *cd;
	struct aic_stats st;
};

#define APPINFO_COL " aid, aname, pid, asn, masn, email, state, tune, "	\
	" to_char(intime, 'YYYY-MM-DD') as intime, "						\
	" to_char(uptime, 'YYYY-MM-DD') as uptime "

#define USERINFO_COL " uid, uname, ip, intime, aid, aname, "	\
	" to_char(intime, 'YYYY-MM-DD') as intime, "				\
	" to_char(uptime, 'YYYY-MM-DD') as uptime "

/*
 * input : aname(STR)
 * return: NORMAL
 * reply : ["state": 0, ...] OR []
 */
static int aic_cmd_appinfo(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL;
	size_t vsize = 0;
	int aid, pid, hit;
	char *aname;

	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	aid = hash_string(aname);

	hit = cache_getf(cd, &val, &vsize, PREFIX_APPINFO"%d", aid);
	if (hit == 0) {
		hdf_set_value(q->hdfsnd, "pname", aname);
		MDB_QUERY_RAW(db, "appinfo", APPINFO_COL, "aid=%d", NULL, aid);
		if (mdb_set_row(q->hdfsnd, db, APPINFO_COL, NULL) == MDB_ERR_NONE) {
			pid = hdf_get_int_value(q->hdfsnd, "pid", 0);
			if (pid != 0) {
				MDB_QUERY_RAW(db, "appinfo", "aname", "aid=%d", NULL, pid);
				mdb_set_row(q->hdfsnd, db, "pname", NULL);
			}
			MDB_QUERY_RAW(db, "appinfo", "COUNT(*)+1 AS numuser", "pid=%d", NULL, aid);
			mdb_set_row(q->hdfsnd, db, "numuser", NULL);
		}
		// cache for error result, because of most be empty, and new/up will del the cache
		//} else {
		//return REP_ERR_DB;
		
		if ((val = calloc(1, MAX_PACKET_LEN)) == NULL) return REP_ERR_MEM;
		vsize = pack_hdf(q->hdfsnd, val, MAX_PACKET_LEN);
		cache_setf(cd, val, vsize, 0, PREFIX_APPINFO"%d", aid);
		free(val);
	} else {
		unpack_hdf(val, vsize, &q->hdfsnd);
	}
	
	return REP_OK;
}

/*
 * input : aname(STR) asn(STR) masn(STR) email(STR) state(INT)
 * return: NORMAL REP_ERR_ALREADYREGIST
 * reply : NULL
 */
static int aic_cmd_appnew(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	char *aname, *pname, *asn, *masn, *email;
	int aid, pid = 0, state, ret;

	REQ_GET_PARAM_INT(q->hdfrcv, "state", state);
	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	REQ_GET_PARAM_STR(q->hdfrcv, "asn", asn);
	REQ_GET_PARAM_STR(q->hdfrcv, "masn", masn);
	REQ_GET_PARAM_STR(q->hdfrcv, "email", email);

	REQ_FETCH_PARAM_STR(q->hdfrcv, "pname", pname);

	aid = hash_string(aname);
	if (pname) pid = hash_string(pname);

	ret = aic_cmd_appinfo(q, cd, db);
	if (PROCESS_NOK(ret)) {
		mtc_err("info get failure %s", aname);
		return ret;
	}

	if (hdf_get_obj(q->hdfsnd, "state")) {
		mtc_warn("%s already regist", aname);
		return REP_ERR_ALREADYREGIST;
	}

	ret = mdb_exec(db, NULL, "INSERT INTO appinfo (aid, aname, "
				   " pid, asn, masn, email, state) "
				   " VALUES ($1, $2, $3, $4, $5, $6, $7);",
				   "isisssi", aid, aname, pid, asn, masn, email, state);
	if (ret != MDB_ERR_NONE) {
		mtc_err("exec failure %s", mdb_get_errmsg(db));
		return REP_ERR_DB;
	}
	
	cache_delf(cd, PREFIX_APPINFO"%d", aid);
	if (pid > 0) {
		cache_delf(cd, PREFIX_APPOUSER"%d_0", pid);
	}

	return REP_OK;
}

/*
 * input : aname(STR) [asn(STR) masn(STR) email(STR) state(INT)]
 * return: NORMAL REP_ERR_ALREADYREGIST
 * reply : NULL
 */
static int aic_cmd_appup(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	char *aname;
	int aid, pid = 0, tune = -1, ret;
	STRING str;

	string_init(&str);
	
	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	REQ_FETCH_PARAM_INT(q->hdfrcv, "tune", tune);
	if (tune != -1) {
		if (hdf_get_int_value(q->hdfrcv, "tuneop", 0)) {
			/* set tune bit */
			string_appendf(&str, " tune=tune | %d, ", tune);
		} else {
			/* unset tune bit */
			string_appendf(&str, " tune=tune & %d, ", ~tune);
		}
	}

	aid = hash_string(aname);
	ret = aic_cmd_appinfo(q, cd, db);
	if (PROCESS_NOK(ret)) {
		mtc_err("info get failure %d", aid);
		return ret;
	}

	if (!hdf_get_obj(q->hdfsnd, "state")) {
		mtc_warn("%d hasn't regist", aid);
		return REP_ERR_NREGIST;
	}
	pid = hdf_get_int_value(q->hdfsnd, "pid", 0);

	mcs_build_upcol_s(q->hdfrcv, hdf_get_child(g_cfg, CONFIG_PATH".appinfo.update.s"), &str);
	mcs_build_upcol_i(q->hdfrcv, hdf_get_child(g_cfg, CONFIG_PATH".appinfo.update.i"), &str);
	if (str.len <= 0) return REP_ERR_BADPARAM;
	string_append(&str, " uptime=uptime ");
	
	ret = mdb_exec(db, NULL, "UPDATE appinfo SET %s WHERE aid=%d;", NULL, str.buf, aid);
	string_clear(&str);
	if (ret != MDB_ERR_NONE) {
		mtc_err("exec failure %s", mdb_get_errmsg(db));
		return REP_ERR_DB;
	}
	
	cache_delf(cd, PREFIX_APPINFO"%d", aid);
	if (pid > 0) {
		cache_delf(cd, PREFIX_APPOUSER"%d_0", pid);
	}

	return REP_OK;
}

/*
 * input : aname(STR)
 * return: NORMAL REP_ERR_NREGIST
 * reply : NULL
 */
static int aic_cmd_appdel(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	char *aname;
	int aid, pid = 0, ret;

	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);

	aid = hash_string(aname);

	ret = aic_cmd_appinfo(q, cd, db);
	if (PROCESS_NOK(ret)) {
		mtc_err("info get failure %s", aname);
		return ret;
	}

	if (!hdf_get_obj(q->hdfsnd, "state")) {
		mtc_warn("%s not regist", aname);
		return REP_ERR_NREGIST;
	}
	pid = hdf_get_int_value(q->hdfsnd, "pid", 0);

	ret = mdb_exec(db, NULL, "DELETE FROM appinfo WHERE aid=%d;", NULL, aid);
	if (ret != MDB_ERR_NONE) {
		mtc_err("exec failure %s", mdb_get_errmsg(db));
		return REP_ERR_DB;
	}
	
	cache_delf(cd, PREFIX_APPINFO"%d", aid);
	if (pid > 0) {
		cache_delf(cd, PREFIX_APPOUSER"%d_0", pid);
	}

	return REP_OK;
}

/*
 * input : aname(STR)
 * return: NORMAL
 * reply : [appfoo: ["aid": "222", "aname": "appfoo", ...]
 */
static int aic_cmd_appusers(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL;
	size_t vsize = 0;
	int aid, hit, count;
	char *aname;

	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	aid = hash_string(aname);
	
	hit = cache_getf(cd, &val, &vsize, PREFIX_USERLIST"%d", aid);
	if (hit == 0) {
		MDB_QUERY_RAW(db, "userinfo", USERINFO_COL,
					  "aid=%d ORDER BY uptime DESC;", NULL, aid);
		mdb_set_rows(q->hdfsnd, db, USERINFO_COL, "userlist", 1);
		
		if ((val = calloc(1, MAX_PACKET_LEN)) == NULL) return REP_ERR_MEM;
		vsize = pack_hdf(q->hdfsnd, val, MAX_PACKET_LEN);
		cache_setf(cd, val, vsize, 0, PREFIX_USERLIST"%d", aid);
		free(val);
	} else {
		unpack_hdf(val, vsize, &q->hdfsnd);
	}
	
	return REP_OK;
}

/*
 * input : uname(STR) aname(STR)
 * return: NORMAL REP_ERR_ALREADYJOIN
 * reply : NULL
 */
static int aic_cmd_appuserin(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	char *uname, *aname, *ip;
	int aid, ret;

	REQ_GET_PARAM_STR(q->hdfrcv, "uname", uname);
	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	REQ_GET_PARAM_STR(q->hdfrcv, "ip", ip);
	aid = hash_string(aname);

	ret = aic_cmd_appusers(q, cd, db);
	if (PROCESS_NOK(ret)) {
		mtc_err("userlist get failure %s", aname);
		return ret;
	}

	if (hdf_get_obj(q->hdfsnd, uname)) {
		mtc_warn("%s already join %s", uname, aname);
		return REP_ERR_ALREADYJOIN;
	}

	ret = mdb_exec(db, NULL, "INSERT INTO userinfo (uid, uname, aid, aname, ip) "
				   " VALUES ($1, $2, $3, $4, $5);", "isiss",
				   hash_string(uname), uname, aid, aname, ip);
	if (ret != MDB_ERR_NONE) {
		mtc_err("exec failure %s", mdb_get_errmsg(db));
		return REP_ERR_DB;
	}

	/* TODO delete aid's ALL cache */
	cache_delf(cd, PREFIX_USERLIST"%d", aid);

	return REP_OK;
}

/*
 * input : uname(STR) aname(STR)
 * return: NORMAL REP_ERR_NOTJOIN
 * reply : NULL
 */
static int aic_cmd_appuserout(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	char *uname, *aname;
	int aid, ret;

	REQ_GET_PARAM_STR(q->hdfrcv, "uname", uname);
	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	aid = hash_string(aname);

	ret = aic_cmd_appusers(q, cd, db);
	if (PROCESS_NOK(ret)) {
		mtc_err("userlist get failure %s", aname);
		return ret;
	}

	if (!hdf_get_obj(q->hdfsnd, uname)) {
		mtc_warn("%s not join %s", uname, aname);
		return REP_ERR_NOTJOIN;
	}

	ret = mdb_exec(db, NULL, "DELETE FROM userinfo WHERE uid=%d AND aid=%d;",
				   NULL, hash_string(uname), aid);
	if (ret != MDB_ERR_NONE) {
		mtc_err("exec failure %s", mdb_get_errmsg(db));
		return REP_ERR_DB;
	}

	/* TODO delete aid's ALL cache */
	cache_delf(cd, PREFIX_USERLIST"%d", aid);

	return REP_OK;
}

/*
 * input : pname(STR)
 * return: NORMAL
 * reply : [appfoo: ["aid": "293029", "aname": "appfoo", ...]
 */
static int aic_cmd_appousers(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL;
	size_t vsize = 0;
	int pid, hit, count, offset;
	char *pname;

	REQ_GET_PARAM_STR(q->hdfrcv, "pname", pname);
	pid = hash_string(pname);
	
	mmisc_get_offset_b(q->hdfsnd, &count, &offset);
	
	hit = cache_getf(cd, &val, &vsize, PREFIX_APPOUSER"%d_%d", pid, offset);
	if (hit == 0) {
		mmisc_set_countf_b(q->hdfsnd, db, "appinfo", "pid=%d OR aid=%d", pid, pid);
		
		MDB_QUERY_RAW(db, "appinfo", APPINFO_COL,
					  "pid=%d OR aid=%d OBDER BY uptime LIMIT %d OFFSET %d",
					  NULL, pid, pid, count, offset);
		mdb_set_rows(q->hdfsnd, db, APPINFO_COL, NULL, 1);
		
		if ((val = calloc(1, MAX_PACKET_LEN)) == NULL) return REP_ERR_MEM;
		vsize = pack_hdf(q->hdfsnd, val, MAX_PACKET_LEN);
		cache_setf(cd, val, vsize, 0, PREFIX_APPOUSER"%d_%d", pid, offset);
		free(val);
	} else {
		unpack_hdf(val, vsize, &q->hdfsnd);
	}
	
	return REP_OK;
}

static void aic_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct aic_entry *e = (struct aic_entry*)entry;
	int ret = REP_OK;
	
	mdb_conn *db = e->db;
	struct cache *cd = e->cd;
	struct aic_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, ret);
	case REQ_CMD_APPINFO:
		ret = aic_cmd_appinfo(q, cd, db);
		break;
	case REQ_CMD_APPNEW:
		ret = aic_cmd_appnew(q, cd, db);
		break;
	case REQ_CMD_APPUP:
		ret = aic_cmd_appup(q, cd, db);
		break;
	case REQ_CMD_APPDEL:
		ret = aic_cmd_appdel(q, cd, db);
		break;
	case REQ_CMD_APPUSERS:
		ret = aic_cmd_appusers(q, cd, db);
		break;
	case REQ_CMD_APPUSERIN:
		ret = aic_cmd_appuserin(q, cd, db);
		break;
	case REQ_CMD_APPUSEROUT:
		ret = aic_cmd_appuserout(q, cd, db);
		break;
	case REQ_CMD_APP_O_USERS:
		ret = aic_cmd_appousers(q, cd, db);
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

static void aic_stop_driver(struct event_entry *entry)
{
	struct aic_entry *e = (struct aic_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	mdb_destroy(e->db);
	cache_free(e->cd);
}



static struct event_entry* aic_init_driver(void)
{
	struct aic_entry *e = calloc(1, sizeof(struct aic_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = aic_process_driver;
	e->base.stop_driver = aic_stop_driver;

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

struct event_driver aic_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = aic_init_driver,
};
