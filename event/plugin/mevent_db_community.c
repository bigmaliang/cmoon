#include "mevent_plugin.h"
#include "mevent_db_community.h"

#define PLUGIN_NAME	"db_community"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

#define MASTER_EVT_NAME	"0"

struct dbcm_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct dbcm_entry {
	struct event_entry base;
	FILE *logf;
	fdb_t *dbhome;
	fdb_t *dbstat;
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
		dtc_err(fp, "exec %s failure %s", c->v.sval.val, fdb_error(db));
	}
	return ret;
}

/*
 * input : sqls(STR/ARRAY)
 * return: NORMAL
 * reply : NULL
 */
static int dbcm_cmd_updatedb(struct queue_entry *q, fdb_t *db, FILE *fp)
{
	struct data_cell *c, *cc;
    int ret;
    
    c = data_cell_search(q->dataset, true, DATA_TYPE_STRING, "sqls");
    if (c != NULL) {
        dtc_dbg(fp, "exec %s ...\n", c->v.sval.val);
        ret = dbcm_exec_cell(c, db, fp);
        if (ret != RET_DBOP_OK) {
            return REP_ERR_DB;
        }
    } else {
        c = data_cell_search(q->dataset, true, DATA_TYPE_ARRAY, "sqls");
    }

    if (c == NULL) {
        dtc_err(fp, "excutable sqls not found\n");
        return REP_ERR_BADPARAM;
    }

    if (c->type == DATA_TYPE_ARRAY) {
		iterate_data(c) {
			cc = c->v.aval->items[t_rsv_i];
            if (cc == NULL) continue;
            if (cc->type == DATA_TYPE_STRING) {
                dtc_dbg(fp, "exec %s ...\n", cc->v.sval.val);
                ret = dbcm_exec_cell(cc, db, fp);
                if (ret != RET_DBOP_OK) {
                    size_t tlen = strlen(MASTER_EVT_NAME) >
                        cc->ksize? strlen(MASTER_EVT_NAME):
                        cc->ksize;
                    if (!strncmp((const char*)cc->key,
							     MASTER_EVT_NAME, tlen)) {
                        return REP_ERR_DB;
                    }
                }
            }
		}
    }
    
    return REP_OK;
}

static void dbcm_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct dbcm_entry *e = (struct dbcm_entry*)entry;
	int ret = REP_OK;
	
	FILE *fp = e->logf;
	fdb_t *dbhome = e->dbhome;
    fdb_t *dbstat = e->dbstat;
    struct cache *cd = NULL;
	struct dbcm_stats *st = &(e->st);

	st->msg_total++;

	dtc_dbg(fp, "process cmd %u", q->operation);
    switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, dbstat, fp, ret);
    case REQ_CMD_STAT:
        ret = dbcm_cmd_updatedb(q, dbstat, fp);
        break;
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
		dtc_err(fp, "process %u failed %d\n", q->operation, ret);
	}
	if (q->req->flags & FLAGS_SYNC) {
        reply_trigger(q, ret);
	}

	if (st->proc_fai >= 1000 && st->proc_fai % 200 == 0) {
		dtc_err(fp, "proc_fai %lu", st->proc_fai);
		SMS_ALARM("db proc_fai %lu", st->proc_fai);
	} else if (st->proc_fai >= 100 && st->proc_fai % 20 == 0) {
		dtc_err(fp, "proc_fai %lu", st->proc_fai);
		SMS_ALARM("db proc_fai %lu", st->proc_fai);
	}

	if (st->msg_unrec == 1000 || st->msg_unrec == 1500) {
		SMS_ALARM("db msg_unrec %lu", st->msg_unrec);
	}
}

static void dbcm_stop_driver(struct event_entry *entry)
{
	struct dbcm_entry *e = (struct dbcm_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	dtc_leave(e->logf);
	fdb_free(&e->dbhome);
    fdb_free(&e->dbstat);
}



static struct event_entry* dbcm_init_driver(void)
{
	struct dbcm_entry *e = calloc(1, sizeof(struct dbcm_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = dbcm_process_driver;
	e->base.stop_driver = dbcm_stop_driver;

	e->logf = dtc_init(hdf_get_value(g_cfg, CONFIG_PATH".logfile",
									 TC_ROOT"plugin/db_community"));
	if (e->logf == NULL) {
		wlog("open log file failure\n");
		goto error;
	}
	
	if (fdb_init_long(&e->dbhome,
                      hdf_get_value(g_cfg, CONFIG_PATH".home.ip", "127.0.0.1"),
					  hdf_get_value(g_cfg, CONFIG_PATH".home.user", "test"),
					  hdf_get_value(g_cfg, CONFIG_PATH".home.pass", "test"),
					  hdf_get_value(g_cfg, CONFIG_PATH".home.name", "static"),
					  (unsigned int)
                      hdf_get_int_value(g_cfg, CONFIG_PATH".home.port", 0))
		!= RET_DBOP_OK) {
		wlog("init %s failure %s\n", PLUGIN_NAME, fdb_error(e->dbhome));
		goto error;
	}
	
	if (fdb_init_long(&e->dbstat,
                      hdf_get_value(g_cfg, CONFIG_PATH".stat.ip", "127.0.0.1"),
					  hdf_get_value(g_cfg, CONFIG_PATH".stat.user", "test"),
					  hdf_get_value(g_cfg, CONFIG_PATH".stat.pass", "test"),
					  hdf_get_value(g_cfg, CONFIG_PATH".stat.name", "static"),
					  (unsigned int)
                      hdf_get_int_value(g_cfg, CONFIG_PATH".stat.port", 0))
		!= RET_DBOP_OK) {
		wlog("init %s failure %s\n", PLUGIN_NAME, fdb_error(e->dbstat));
		goto error;
	}
	
	return (struct event_entry*)e;
	
 error:
	if (e->base.name) free(e->base.name);
	if (e->logf) dtc_leave(e->logf);
	if (e->dbhome) fdb_free(&e->dbhome);
	if (e->dbstat) fdb_free(&e->dbstat);
	free(e);
	return NULL;
}

struct event_driver db_community_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = dbcm_init_driver,
};
