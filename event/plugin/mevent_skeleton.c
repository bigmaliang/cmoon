#include "mevent_plugin.h"
#include "mevent_skeleton.h"

#define PLUGIN_NAME	"skeleton"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct skeleton_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct skeleton_entry {
	struct event_entry base;
	mdb_conn *db;
	struct cache *cd;
	struct skeleton_stats st;
};

static void skeleton_process_driver(struct event_entry *entry, QueueEntry *q)
{
	struct skeleton_entry *e = (struct skeleton_entry*)entry;
	NEOERR *err;
	int ret;
	
	mdb_conn *db = e->db;
	struct cache *cd = e->cd;
	struct skeleton_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, e->cd, err);
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

static void skeleton_stop_driver(struct event_entry *entry)
{
	struct skeleton_entry *e = (struct skeleton_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	mdb_destroy(e->db);
	cache_free(e->cd);
}



static struct event_entry* skeleton_init_driver(void)
{
	struct skeleton_entry *e = calloc(1, sizeof(struct skeleton_entry));
	if (e == NULL) return NULL;
	NEOERR *err;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = skeleton_process_driver;
	e->base.stop_driver = skeleton_stop_driver;

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

struct event_driver skeleton_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = skeleton_init_driver,
};
