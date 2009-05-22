#include "mevent_plugin.h"

#include "fdb.h"

#define PLUGIN_NAME	"db_community"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

#define DBCM_IP		"192.168.8.84"
#define DBCM_USER	"root"
#define DBCM_PASS	"hifly1234"
#define DBCM_NAME 	"home"

#define MASTER_EVT_NAME	"0"

struct dbcm_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct dbcm_entry {
	struct event_entry base;
	FILE *logf;
	fdb_t *db;
	struct dbcm_stats st;
};

static int dbcm_exec_cell(struct data_cell *c, fdb_t *db, FILE *fp)
{
	int ret;
	size_t len = c->v.sval.vsize < sizeof(db->sql) ? c->v.sval.vsize: sizeof(db->sql);
	memset(db->sql, 0x0, sizeof(db->sql));
	strncpy(db->sql, (const char*)c->v.sval.val, len);
	ret = fdb_exec(db);
	if (ret != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s\n", c->v.sval.val, fdb_error(db));
	}
	return ret;
}

static void dbcm_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct dbcm_entry *e;
	struct data_cell *c, *cc;
	int ret = RET_DBOP_OK;
	
	e = (struct dbcm_entry*)entry;
	FILE *fp = e->logf;
	fdb_t *db = e->db;
	struct dbcm_stats st = e->st;

	st.msg_total++;

	usleep(300);
	if (q->operation != REQ_CMD_STATS) {
		c = data_cell_search(q->dataset, true, DATA_TYPE_STRING, "sqls");
		if (c != NULL) {
			ret = dbcm_exec_cell(c, db, fp);
			if (ret != RET_DBOP_OK)	st.proc_fai++;
			else st.proc_suc++;
		} else {
			c = data_cell_search(q->dataset, true, DATA_TYPE_ARRAY, "sqls");
		}

		if (c == NULL) {
			st.msg_unrec++;
			dtc_err(fp, "excutable sqls not found\n");
		}

		if (c != NULL && c->type == DATA_TYPE_ARRAY) {
			data_cell_array_iterate(c, cc) {
				if (cc == NULL) continue;
				if (cc->type == DATA_TYPE_STRING) {
					dtc_dbg(fp, "exec %s ...\n", cc->v.sval.val);
					ret = dbcm_exec_cell(cc, db, fp);
					if (ret != RET_DBOP_OK) {
						st.proc_fai++;
						size_t tlen = strlen(MASTER_EVT_NAME) >
							cc->ksize? strlen(MASTER_EVT_NAME):
							cc->ksize;
						if (!strncmp((const char*)cc->key,
							     MASTER_EVT_NAME, tlen)) {
							break;
						}
					}
					else st.proc_suc++;
				}
			}
		}
	} else {
		st.msg_stats++;
		if ((q->req->flags & FLAGS_SYNC)) {
			reply_add_ulong(q, NULL, "msg_total", st.msg_total);
			reply_add_ulong(q, NULL, "msg_unrec", st.msg_unrec);
			reply_add_ulong(q, NULL, "msg_stats", st.msg_stats);
			reply_add_ulong(q, NULL, "proc_suc", st.proc_suc);
			reply_add_ulong(q, NULL, "proc_fai", st.proc_fai);
			reply_trigger(q, REP_OK);
		}
	}
	
	if ((q->req->flags & FLAGS_SYNC)) {
		if (ret == RET_DBOP_OK) {
			q->req->reply_mini(q->req, REP_OK);
		} else {
			q->req->reply_err(q->req, ERR_DB);
		}
	}
}

static void dbcm_stop_driver(struct event_entry *entry)
{
	struct dbcm_entry *e = (struct dbcm_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	dtc_leave(e->logf);
	fdb_free(&e->db);
}



static struct event_entry* dbcm_init_driver(void)
{
	struct dbcm_entry *e = calloc(1, sizeof(struct dbcm_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = dbcm_process_driver;
	e->base.stop_driver = dbcm_stop_driver;

	e->logf = dtc_init(hdf_get_value(g_cfg, CONFIG_PATH".logfile", TC_ROOT"plugin/db_community"));
	if (e->logf == NULL) {
		wlog("open log file %splugin/db_community.log failure\n", TC_ROOT);
		goto error;
	}
	
	if (fdb_init_long(&e->db, hdf_get_value(g_cfg, CONFIG_PATH".ip", DBCM_IP),
			  hdf_get_value(g_cfg, CONFIG_PATH".user", DBCM_USER),
			  hdf_get_value(g_cfg, CONFIG_PATH".pass", DBCM_PASS),
			  hdf_get_value(g_cfg, CONFIG_PATH".name", DBCM_NAME)) != RET_DBOP_OK) {
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

struct event_driver db_community_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = dbcm_init_driver,
};
