#include "mevent_plugin.h"
#include "mevent_mtls.h"

#define PLUGIN_NAME	"mtls"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct mtls_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct mtls_entry {
	struct event_entry base;
	mdb_conn *db;
	struct cache *cd;
	struct mtls_stats st;
};

#define COL_VISIT	" aid, pv, uv, date_part('epoch', dt)*1000 AS intime "
#define COL_REFER	" refer, SUM(count) AS count "
#define COL_URL		" url, title, SUM(count) AS count "
#define COL_AREA	" area, SUM(count) AS count "

static NEOERR* mtls_cmd_getstat(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	STRING str; string_init(&str);
	unsigned char *val = NULL; size_t vsize = 0;
	char *aname;
	int aid;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	aid = hash_string(aname);
	hdf_set_int_value(q->hdfrcv, "aid", aid);

	err = mcs_build_querycond(q->hdfrcv,
							  hdf_get_obj(g_cfg, CONFIG_PATH".QueryCond.stat"),
							  &str, " dt > current_date - 7 ");
	if (err != STATUS_OK) return nerr_pass(err);

	if (cache_getf(cd, &val, &vsize, PREFIX_MTLS"%s", str.buf)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		MDB_QUERY_RAW(db, "visit", COL_VISIT, "%s ORDER BY dt", NULL, str.buf);
		err = mdb_set_rows(q->hdfsnd, db, COL_VISIT, "visit", -1);
		if (err != STATUS_OK) return nerr_pass(err);

		MDB_QUERY_RAW(db, "topref", COL_REFER, "%s GROUP BY refer", NULL, str.buf);
		err = mdb_set_rows(q->hdfsnd, db, COL_REFER, "refer", -1);
		if (err != STATUS_OK) return nerr_pass(err);

		MDB_QUERY_RAW(db, "topurl", COL_URL, "%s GROUP BY url, title", NULL, str.buf);
		err = mdb_set_rows(q->hdfsnd, db, COL_URL, "url", -1);
		if (err != STATUS_OK) return nerr_pass(err);

		MDB_QUERY_RAW(db, "toparea", COL_AREA, "%s GROUP BY area", NULL, str.buf);
		err = mdb_set_rows(q->hdfsnd, db, COL_AREA, "area", -1);
		if (err != STATUS_OK) return nerr_pass(err);

		CACHE_HDF(q->hdfsnd, ONE_HOUR, PREFIX_MTLS"%s", str.buf);
	}

	string_clear(&str);

	return STATUS_OK;
}

static void mtls_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct mtls_entry *e = (struct mtls_entry*)entry;
	NEOERR *err;
	int ret;
	
	mdb_conn *db = e->db;
	struct cache *cd = e->cd;
	struct mtls_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, e->cd, err);
	case REQ_CMD_GETSTAT:
		err = mtls_cmd_getstat(q, cd, db);
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

static void mtls_stop_driver(struct event_entry *entry)
{
	struct mtls_entry *e = (struct mtls_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	mdb_destroy(e->db);
	cache_free(e->cd);
}



static struct event_entry* mtls_init_driver(void)
{
	struct mtls_entry *e = calloc(1, sizeof(struct mtls_entry));
	if (e == NULL) return NULL;
	NEOERR *err;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = mtls_process_driver;
	e->base.stop_driver = mtls_stop_driver;

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

struct event_driver mtls_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = mtls_init_driver,
};
