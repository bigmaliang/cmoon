#include "mevent_plugin.h"

#define PLUGIN_NAME	"skeleton2"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct skeleton2_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct skeleton2_entry {
	struct event_entry base;
	FILE *logf;
	fdb_t *db;
	struct skeleton2_stats st;
};

static void skeleton2_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct skeleton2_entry *e = (struct skeleton2_entry*)entry;
	struct data_cell *c, *cc;
	int ret = RET_DBOP_OK;
	
	e = (struct skeleton2_entry*)entry;
	FILE *fp = e->logf;
	fdb_t *db = e->db;
	struct skeleton2_stats *st = &(e->st);

	st->msg_total++;
	
	if (q->operation == REQ_CMD_STATS) {
		st->msg_stats++;
		if ((q->req->flags & FLAGS_SYNC)) {
			reply_add_ulong(q, NULL, "msg_total", st->msg_total);
			reply_add_ulong(q, NULL, "msg_unrec", st->msg_unrec);
			reply_add_ulong(q, NULL, "msg_stats", st->msg_stats);
			reply_add_ulong(q, NULL, "proc_suc", st->proc_suc);
			reply_add_ulong(q, NULL, "proc_fai", st->proc_fai);
			reply_trigger(q, REP_OK);
			return;
		}
	}
	
	if ((q->req->flags & FLAGS_SYNC)) {
		if (ret == RET_DBOP_OK) {
			q->req->reply_mini(q->req, REP_OK);
		} else {
			q->req->reply_err(q->req, ERR_DB);
		}
		return;
	}
}

static void skeleton2_stop_driver(struct event_entry *entry)
{
	struct skeleton2_entry *e = (struct skeleton2_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	dtc_leave(e->logf);
	fdb_free(&e->db);
}



static struct event_entry* skeleton2_init_driver(void)
{
	struct skeleton2_entry *e = calloc(1, sizeof(struct skeleton2_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = skeleton2_process_driver;
	e->base.stop_driver = skeleton2_stop_driver;

	e->logf = dtc_init(hdf_get_value(g_cfg, CONFIG_PATH".logfile", TC_ROOT"plugin/skeleton2"));
	if (e->logf == NULL) {
		wlog("open log file %splugin/db_community.log failure\n", TC_ROOT);
		goto error;
	}
	
	if (fdb_init_long(&e->db, hdf_get_value(g_cfg, CONFIG_PATH".ip", "127.0.0.1"),
					  hdf_get_value(g_cfg, CONFIG_PATH".user", "test"),
					  hdf_get_value(g_cfg, CONFIG_PATH".pass", "test"),
					  hdf_get_value(g_cfg, CONFIG_PATH".name", "user_info"),
					  (unsigned int)hdf_get_int_value(g_cfg, CONFIG_PATH".port", 0)) != RET_DBOP_OK) {
		wlog("init %s failure %s\n", PLUGIN_NAME, fdb_error(e->db));
		goto error;
	}
	
	return (struct event_entry*)e;
	
 error:
	if (e->base.name) free(e->base.name);
	if (e->logf) dtc_leave(e->logf);
	if (e->db) fdb_free(&e->db);
	free(e);
	return NULL;
}

struct event_driver skeleton2_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = skeleton2_init_driver,
};
