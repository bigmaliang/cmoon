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

/*
 * input : aname(STR)
 * return: NORMAL
 * reply : ["state": 0, ...] OR []
 */
static int aic_cmd_appinfo(struct queue_entry *q, struct cache *cd,
						   mdb_conn *db)
{
	unsigned char *val = NULL;
	size_t vsize = 0;
	int aid, pid, state, hit, ret;
	char *aname, *masn, *email;

	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	aid = hash_string(aname);
	
	hit = cache_getf(cd, &val, &vsize, PREFIX_AIC"%d", aid);
	if (hit == 0) {
		ret = mdb_exec(db, NULL, "SELECT aid, aname, pid, masn, email, state FROM "
					   " appinfo WHERE aid=%d;", NULL, aid);
		if (ret != MDB_ERR_NONE) {
			mtc_err("exec failure %s", mdb_get_errmsg(db));
			return REP_ERR_DB;
		}
		while (mdb_get(db, "isissi", &aid, &aname, &pid, &masn, &email, &state)
			   == MDB_ERR_NONE) {
			hdf_set_int_value(q->hdfsnd, "aid", aid);
			hdf_set_value(q->hdfsnd, "aname", aname);
			hdf_set_int_value(q->hdfsnd, "pid", pid);
			hdf_set_value(q->hdfsnd, "masn", masn);
			hdf_set_value(q->hdfsnd, "email", email);
			hdf_set_int_value(q->hdfsnd, "state", state);
		}
		val = calloc(1, MAX_PACKET_LEN);
		if (val == NULL) {
			return REP_ERR_MEM;
		}
		vsize = pack_hdf(q->hdfsnd, val);
		cache_setf(cd, val, vsize, PREFIX_AIC"%d", aid);
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
static int aic_cmd_appnew(struct queue_entry *q, struct cache *cd,
						  mdb_conn *db)
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
	
	cache_delf(cd, PREFIX_AIC"%d", aid);

	return REP_OK;
}

/*
 * input : aname(STR) [asn(STR) masn(STR) email(STR) state(INT)]
 * return: NORMAL REP_ERR_ALREADYREGIST
 * reply : NULL
 */
static int aic_cmd_appup(struct queue_entry *q, struct cache *cd,
						 mdb_conn *db)
{
	char *aname, *asn, *masn, *email;
	int aid, state = -1, ret;

	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	REQ_FETCH_PARAM_STR(q->hdfrcv, "asn", asn);
	REQ_FETCH_PARAM_STR(q->hdfrcv, "masn", masn);
	REQ_FETCH_PARAM_STR(q->hdfrcv, "email", email);
	REQ_FETCH_PARAM_INT(q->hdfrcv, "state", state);

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

	char cols[1024], tok[128];
	strcpy(cols, "");
	if (aname) {
		snprintf(tok, sizeof(tok), " aname='%s", aname);
		strcat(cols, tok);
		/* avoid strlen(aname) > sizeof(tok) */
		strcat(cols, "', ");
	}
	if (asn) {
		snprintf(tok, sizeof(tok), " asn='%s", asn);
		strcat(cols, tok); strcat(cols, "', ");
	}
	if (masn) {
		snprintf(tok, sizeof(tok), " masn='%s", masn);
		strcat(cols, tok); strcat(cols, "', ");
	}
	if (email) {
		snprintf(tok, sizeof(tok), " email='%s", email);
		strcat(cols, tok); strcat(cols, "', ");
	}
	if (state != -1) {
		snprintf(tok, sizeof(tok), " state=%d, ", state);
		strcat(cols, tok);
	}

	if (!strcmp(cols, "")) {
		return REP_ERR_BADPARAM;
	}
	
	ret = mdb_exec(db, NULL, "UPDATE appinfo SET %s WHERE aid=%d;", NULL, cols, aid);
	if (ret != MDB_ERR_NONE) {
		mtc_err("exec failure %s", mdb_get_errmsg(db));
		return REP_ERR_DB;
	}
	
	cache_delf(cd, PREFIX_AIC"%d", aid);

	return REP_OK;
}

/*
 * input : aname(STR)
 * return: NORMAL
 * reply : ["293029": ["aid": "222", "aname": "appfoo", ...]
 */
static int aic_cmd_appuserlist(struct queue_entry *q, struct cache *cd,
							   mdb_conn *db)
{
	unsigned char *val = NULL;
	size_t vsize = 0;
	int aid, uid, hit, ret;
	char *aname, *uname, *intime;

	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	aid = hash_string(aname);
	
	hit = cache_getf(cd, &val, &vsize, PREFIX_USERLIST"%d", aid);
	if (hit == 0) {
		ret = mdb_exec(db, NULL, "SELECT uid, uname, intime, aid, aname FROM "
					   " userinfo WHERE aid=%d;", NULL, aid);
		if (ret != MDB_ERR_NONE) {
			mtc_err("exec failure %s", mdb_get_errmsg(db));
			return REP_ERR_DB;
		}
		while (mdb_get(db, "iss", &uid, &uname, &intime)
			   == MDB_ERR_NONE) {
			hdf_set_valuef(q->hdfsnd, "%s.aid=%d", uname, aid);
			hdf_set_valuef(q->hdfsnd, "%s.aname=%s", uname, aname);
			hdf_set_valuef(q->hdfsnd, "%s.uid=%d", uname, uid);
			hdf_set_valuef(q->hdfsnd, "%s.intime=%s", uname, intime);
		}
		val = calloc(1, MAX_PACKET_LEN);
		if (val == NULL) {
			return REP_ERR_MEM;
		}
		vsize = pack_hdf(q->hdfsnd, val);
		cache_setf(cd, val, vsize, PREFIX_USERLIST"%d", aid);
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
static int aic_cmd_appuserjoin(struct queue_entry *q, struct cache *cd,
							   mdb_conn *db)
{
	char *uname, *aname;
	int aid, ret;

	REQ_GET_PARAM_STR(q->hdfrcv, "uname", uname);
	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	aid = hash_string(aname);

	ret = aic_cmd_appuserlist(q, cd, db);
	if (PROCESS_NOK(ret)) {
		mtc_err("userlist get failure %s", aname);
		return ret;
	}

	if (hdf_get_obj(q->hdfsnd, uname)) {
		mtc_warn("%d already join", aid);
		return REP_ERR_ALREADYJOIN;
	}

	ret = mdb_exec(db, NULL, "INSERT INTO userinfo (uid, uname, aid, aname) "
				   " VALUES ($1, $2, $3, $4);", "isis",
				   hash_string(uname), uname, aid, aname);
	if (ret != MDB_ERR_NONE) {
		mtc_err("exec failure %s", mdb_get_errmsg(db));
		return REP_ERR_DB;
	}
	
	cache_delf(cd, PREFIX_USERLIST"%d", aid);

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
	case REQ_CMD_USERLIST:
		ret = aic_cmd_appuserlist(q, cd, db);
		break;
	case REQ_CMD_USERJOIN:
		ret = aic_cmd_appuserjoin(q, cd, db);
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
