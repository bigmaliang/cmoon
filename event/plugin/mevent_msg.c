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

static NEOERR* msg_cmd_mysaid(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	int count, offset;
	char *name;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "name", name);

	mmisc_pagediv_get(q->hdfrcv, NULL, &count, &offset);
	
	if (cache_getf(cd, &val, &vsize, PREFIX_MYSAID"%s_%d", name, offset)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		MMISC_PAGEDIV_SET_N(q->hdfsnd, offset, db, "msg", "mfrom=$1", "s", name);
		MDB_QUERY_RAW(db, "msg", MSG_COL, "mfrom=$1 ORDER BY intime DESC "
					  " LIMIT %d OFFSET %d", "s", count, offset, name);
		err = mdb_set_rows(q->hdfsnd, db, NULL, "raws", -1);
		if (err != STATUS_OK) return nerr_pass(err);
		CACHE_HDF(q->hdfsnd, MSG_CC_SEC, PREFIX_MYSAID"%s_%d", name, offset);
	}

	return STATUS_OK;
}

static NEOERR* msg_cmd_saytome(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	int count, offset;
	char *name;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "name", name);

	mmisc_pagediv_get(q->hdfrcv, NULL, &count, &offset);

	if (cache_getf(cd, &val, &vsize, PREFIX_SAYTOME"%s_%d", name, offset)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		MMISC_PAGEDIV_SET_N(q->hdfsnd, offset, db, "msg", "mto=$1", "s", name);
		MDB_QUERY_RAW(db, "msg", MSG_COL, "mto=$1 ORDER BY intime DESC "
					  " LIMIT %d OFFSET %d", "s", count, offset, name);
		err = mdb_set_rows(q->hdfsnd, db, NULL, "raws", -1);
		if (err != STATUS_OK) return nerr_pass(err);
		CACHE_HDF(q->hdfsnd, MSG_CC_SEC, PREFIX_SAYTOME"%s_%d", name, offset);
	}

	return STATUS_OK;
}

static NEOERR* msg_cmd_saywithother(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	int count, offset;
	char *name, *name2, key[LEN_MD];
	NEOERR *err;

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
							"(mfrom=$1 AND mto=$2) OR (mfrom=$3 AND mto=$4)",
							"ssss", name, name2, name2, name);
		MDB_QUERY_RAW(db, "msg", MSG_COL, "(mfrom=$1 AND mto=$2) OR "
					  " (mfrom=$3 AND mto=$4)"
					  " ORDER BY intime DESC LIMIT %d OFFSET %d",
					  "ssss", count, offset, name, name2, name2, name);
		err = mdb_set_rows(q->hdfsnd, db, NULL, "raws", -1);
		if (err != STATUS_OK) return nerr_pass(err);
		CACHE_HDF(q->hdfsnd, MSG_CC_SEC, PREFIX_SAYWITHOTHER"%s_%d", key, offset);
	}

	return STATUS_OK;
}

static NEOERR* msg_cmd_mymsg(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	int count, offset;
	char *name;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "name", name);

	mmisc_pagediv_get(q->hdfrcv, NULL, &count, &offset);

	if (cache_getf(cd, &val, &vsize, PREFIX_MYMSG"%s_%d", name, offset)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		MMISC_PAGEDIV_SET_N(q->hdfsnd, offset, db, "msg", "mfrom=$1 OR mto=$2"
							"ss", name, name);
		MDB_QUERY_RAW(db, "msg", MSG_COL, "mfrom=$1 OR mto=$2"
					  " ORDER BY intime DESC LIMIT %d OFFSET %d",
					  "ss", count, offset, name, name);
		err = mdb_set_rows(q->hdfsnd, db, NULL, "raws", -1);
		if (err != STATUS_OK) return nerr_pass(err);
		CACHE_HDF(q->hdfsnd, MSG_CC_SEC, PREFIX_MYMSG"%s_%d", name, offset);
	}
	
	return STATUS_OK;
}

static NEOERR* msg_cmd_msgset(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	char *from, *to, *raw;
	int type;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "from", from);
	REQ_GET_PARAM_STR(q->hdfrcv, "to", to);
	REQ_GET_PARAM_STR(q->hdfrcv, "raw", raw);
	REQ_GET_PARAM_INT(q->hdfrcv, "type", type);

	MDB_EXEC(db, NULL, "INSERT INTO msg (mfrom, mto, mtype, mraw) "
			 " VALUES ($1::varchar(256), $2::varchar(256), "
			 " $3, $4::varchar(1024));", "ssis", from, to, type, raw);
	
	/* cache removed by timeout, not here */
	
	return STATUS_OK;
}

static NEOERR* msg_cmd_del_mysaid(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	char *name;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "name", name);

	MDB_EXEC(db, NULL, "DELETE FROM msg WHERE mfrom=$1", "s", name);

	return STATUS_OK;
}

static NEOERR* msg_cmd_del_saytome(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	char *name;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "name", name);

	MDB_EXEC(db, NULL, "DELETE FROM msg WHERE mto=$1", "s", name);

	return STATUS_OK;
}

static NEOERR* msg_cmd_del_both(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	char *name, *name2;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "name", name);
	REQ_GET_PARAM_STR(q->hdfrcv, "name2", name2);
	
	MDB_EXEC(db, NULL, "DELETE FROM msg WHERE "
			 "(mfrom=$1 AND mto=$2) OR (mfrom=$3 AND mto=$4) ",
			 "ssss", name, name2, name2, name);
	
	return STATUS_OK;
}

static void msg_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct msg_entry *e = (struct msg_entry*)entry;
	NEOERR *err;
	int ret;
	
	mdb_conn *db = e->db;
	struct cache *cd = e->cd;
	struct msg_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, err);
	case REQ_CMD_MYMSG:
		err = msg_cmd_mymsg(q, cd, db);
		break;
	case REQ_CMD_MYSAID:
		err = msg_cmd_mysaid(q, cd, db);
		break;
	case REQ_CMD_SAYTOME:
		err = msg_cmd_saytome(q, cd, db);
		break;
	case REQ_CMD_SAYWITHOTHER:
		err = msg_cmd_saywithother(q, cd, db);
		break;
	case REQ_CMD_MSGSET:
		err = msg_cmd_msgset(q, cd, db);
		break;
	case REQ_CMD_DEL_MYSAID:
		err = msg_cmd_del_mysaid(q, cd, db);
		break;
	case REQ_CMD_DEL_SAYTOME:
		err = msg_cmd_del_saytome(q, cd, db);
		break;
	case REQ_CMD_DEL_BOTH:
		err = msg_cmd_del_both(q, cd, db);
		break;
	case REQ_CMD_STATS:
		st->msg_stats++;
		err = STATUS_OK;
		hdf_set_int_value(q->hdfsnd, "msg_total", st->msg_total);
		hdf_set_int_value(q->hdfsnd, "msg_unrec", st->msg_unrec);
		hdf_set_int_value(q->hdfsnd, "msg_badparam", st->msg_badparam);
		hdf_set_int_value(q->hdfsnd, "msg_stats", st->msg_stats);
		hdf_set_int_value(q->hdfsnd, "proc_suc", st->proc_suc);
		hdf_set_int_value(q->hdfsnd, "proc_fai", st->proc_fai);
		break;
	default:
		st->msg_unrec++;
		err = nerr_raise(REP_ERR_UNKREQ, "unknown command %u", q->operation);
		break;
	}
	
	NEOERR *neede = mcs_err_valid(err);
	ret = neede ? neede->error : REP_OK;
	if (PROCESS_OK(ret)) {
		st->proc_suc++;
	} else {
		st->proc_fai++;
		if (ret == REP_ERR_BADPARAM) {
			st->msg_badparam++;
		}
		TRACE_ERR(q, ret, err);
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
	NEOERR *err;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = msg_process_driver;
	e->base.stop_driver = msg_stop_driver;

	char *dbsn = hdf_get_value(g_cfg, CONFIG_PATH".dbsn", NULL);
	err = mdb_init(&e->db, dbsn);
	JUMP_NOK(err, error);
	
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
