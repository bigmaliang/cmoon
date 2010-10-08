#include "mevent_plugin.h"
#include "mevent_msg.h"

#define PLUGIN_NAME	"msg"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct msg_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct msg_entry {
	struct event_entry base;
	mdb_conn *db;
	struct cache *cd;
	struct msg_stats st;
};

#define MSG_COL " mraw "

static int msg_cmd_mysaid(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	int count, offset;
	char *name;

	REQ_GET_PARAM_STR(q->hdfrcv, "name", name);

	mmisc_pagediv_get(q->hdfrcv, NULL, &count, &offset);
	
	if (cache_getf(cd, &val, &vsize, PREFIX_MYSAID"%s_%d", name, offset)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		MMISC_PAGEDIV_SET_N(q->hdfsnd, offset, db, "msg", "mfrom=$1", "s", name);
		MDB_QUERY_RAW(db, "msg", MSG_COL, "mfrom=$1", "s", name);
		mdb_set_rows(q->hdfsnd, db, NULL, "raws", -1);
		CACHE_HDF(q->hdfsnd, 600, PREFIX_MYSAID"%s_%d", name, offset);
	}

	return REP_OK;
}

static int msg_cmd_saytome(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	int count, offset;
	char *name;

	REQ_GET_PARAM_STR(q->hdfrcv, "name", name);

	mmisc_pagediv_get(q->hdfrcv, NULL, &count, &offset);

	if (cache_getf(cd, &val, &vsize, PREFIX_SAYTOME"%s_%d", name, offset)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		MMISC_PAGEDIV_SET_N(q->hdfsnd, offset, db, "msg", "mto=$1", "s", name);
		MDB_QUERY_RAW(db, "msg", MSG_COL, "mto=$1", "s", name);
		mdb_set_rows(q->hdfsnd, db, NULL, "raws", -1);
		CACHE_HDF(q->hdfsnd, 600, PREFIX_SAYTOME"%s_%d", name, offset);
	}

	return REP_OK;
}

static int msg_cmd_saywithother(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	int count, offset;
	char *name, *name2, key[LEN_MD];

	REQ_GET_PARAM_STR(q->hdfrcv, "name", name);
	REQ_GET_PARAM_STR(q->hdfrcv, "name2", name2);

	mmisc_pagediv_get(q->hdfrcv, NULL, &count, &offset);

	if (strcmp(name, name2) > 0) {
		snprintf(key, sizeof(key), "%s%s", name, name2);
	} else {
		snprintf(key, sizeof(key), "%s%s", name2, name);
	}

	if (cache_getf(cd, &val, &vsize, PREFIX_SAYWITHOTHER"%s_%d", key, offset)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		MMISC_PAGEDIV_SET_N(q->hdfsnd, offset, db, "msg",
							"(mfrom=%s AND mto=%s) OR (mfrom=%s AND mto=%s)",
							name, name2, name2, name);
		MDB_QUERY_RAW(db, "msg", MSG_COL, "(mfrom=$1 AND mto=$2) OR (mfrom=$3 AND mto=$4)",
					  "ssss", name, name2, name2, name);
		mdb_set_rows(q->hdfsnd, db, NULL, "raws", -1);
		CACHE_HDF(q->hdfsnd, 600, PREFIX_SAYWITHOTHER"%s_%d", key, offset);
	}

	return REP_OK;
}

static int msg_cmd_mymsg(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	int count, offset;
	char *name;

	REQ_GET_PARAM_STR(q->hdfrcv, "name", name);

	mmisc_pagediv_get(q->hdfrcv, NULL, &count, &offset);

	if (cache_getf(cd, &val, &vsize, PREFIX_MYMSG"%s_%d", name, offset)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		MMISC_PAGEDIV_SET_N(q->hdfsnd, offset, db, "msg", "mfrom=$1 OR mto=$2"
							"ss", name, name);
		MDB_QUERY_RAW(db, "msg", MSG_COL, "mfrom=$1 OR mto=$2",
					  "ss", name, name);
		mdb_set_rows(q->hdfsnd, db, NULL, "raws", -1);
		CACHE_HDF(q->hdfsnd, 600, PREFIX_MYMSG"%s_%d", name, offset);
	}
	
	return REP_OK;
}

static int msg_cmd_msgset(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	char *from, *to, *raw;
	int type;

	REQ_GET_PARAM_STR(q->hdfrcv, "from", from);
	REQ_GET_PARAM_STR(q->hdfrcv, "to", to);
	REQ_GET_PARAM_STR(q->hdfrcv, "raw", raw);
	REQ_GET_PARAM_INT(q->hdfrcv, "type", type);

	MDB_EXEC_EVT(db, NULL, "INSERT INTO msg (mfrom, mto, mtype, mraw) "
				 " VALUES ($1, $2, $3, $4);", "ssis", from, to, type, raw);

	/* cache removed by timeout, not here */

	return REP_OK;
}

static void msg_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct msg_entry *e = (struct msg_entry*)entry;
	int ret = REP_OK;
	
	mdb_conn *db = e->db;
	struct cache *cd = e->cd;
	struct msg_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, ret);
	case REQ_CMD_MYMSG:
		ret = msg_cmd_mymsg(q, cd, db);
		break;
	case REQ_CMD_MYSAID:
		ret = msg_cmd_mysaid(q, cd, db);
		break;
	case REQ_CMD_SAYTOME:
		ret = msg_cmd_saytome(q, cd, db);
		break;
	case REQ_CMD_SAYWITHOTHER:
		ret = msg_cmd_saywithother(q, cd, db);
		break;
	case REQ_CMD_MSGSET:
		ret = msg_cmd_msgset(q, cd, db);
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

static void msg_stop_driver(struct event_entry *entry)
{
	struct msg_entry *e = (struct msg_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	mdb_destroy(e->db);
	cache_free(e->cd);
}



static struct event_entry* msg_init_driver(void)
{
	struct msg_entry *e = calloc(1, sizeof(struct msg_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = msg_process_driver;
	e->base.stop_driver = msg_stop_driver;

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

struct event_driver msg_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = msg_init_driver,
};
