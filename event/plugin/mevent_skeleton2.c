#include "mevent_plugin.h"
#include "mevent_skeleton2.h"

#define PLUGIN_NAME	"skeleton2"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct skeleton2_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct skeleton2_entry {
	struct event_entry base;
	mdb_conn *db;
	struct cache *cd;
	struct skeleton2_stats st;
};

static void skeleton2_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct skeleton2_entry *e = (struct skeleton2_entry*)entry;
	int ret = REP_OK;
	
	mdb_conn *db = e->db;
	struct cache *cd = e->cd;
	struct skeleton2_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, ret);
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
		mtc_err("process %u failed %d", q->operation, ret);
	}
	if (q->req->flags & FLAGS_SYNC) {
			reply_trigger(q, ret);
	}
}

static void skeleton2_stop_driver(struct event_entry *entry)
{
	struct skeleton2_entry *e = (struct skeleton2_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	mdb_destroy(e->db);
	cache_free(e->cd);
}



static struct event_entry* skeleton2_init_driver(void)
{
	struct skeleton2_entry *e = calloc(1, sizeof(struct skeleton2_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = skeleton2_process_driver;
	e->base.stop_driver = skeleton2_stop_driver;

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

struct event_driver skeleton2_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = skeleton2_init_driver,
};
