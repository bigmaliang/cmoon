#include "mevent_plugin.h"
#include "mevent_rawdb.h"

#define PLUGIN_NAME	"rawdb"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

#define MASTER_EVT_NAME	"0"

struct rawdb_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct rawdb_entry {
	struct event_entry base;
	mdb_conn *dbaccess;
	mdb_conn *dbstat;
	struct cache *cd;
	struct rawdb_stats st;
};

static int rawdb_exec_cell(struct data_cell *c, mdb_conn *db)
{
	int ret = mdb_exec(db, NULL, (char*)c->v.sval.val, NULL);
	if (ret != MDB_ERR_NONE) {
		mtc_err("exec %s failure %s", c->v.sval.val, mdb_get_errmsg(db));
	} else {
		if (strncasecmp(c->v.sval.val, "insert", strlen("insert"))) {
			reply_add_u32(q, NULL, "id", mdb_get_last_id(db, NULL));
		}
	}
	return ret;
}

/*
 * input : sqls(STR/ARRAY)
 * return: NORMAL
 * reply : NULL
 */
static int rawdb_cmd_updatedb(struct queue_entry *q, mdb_conn *db)
{
	struct data_cell *c, *cc;
    int ret;
    
    c = data_cell_search(q->dataset, true, DATA_TYPE_STRING, "sqls");
    if (c != NULL) {
        mtc_dbg("exec %s ...\n", c->v.sval.val);
        ret = rawdb_exec_cell(c, db);
        if (ret != MDB_ERR_NONE) {
            return REP_ERR_DB;
        }
    } else {
        c = data_cell_search(q->dataset, true, DATA_TYPE_ARRAY, "sqls");
    }

    if (c == NULL) {
        mtc_err("excutable sqls not found\n");
        return REP_ERR_BADPARAM;
    }

    if (c->type == DATA_TYPE_ARRAY) {
		iterate_data(c) {
			cc = c->v.aval->items[t_rsv_i];
            if (cc == NULL) continue;
            if (cc->type == DATA_TYPE_STRING) {
                mtc_dbg("exec %s ...\n", cc->v.sval.val);
                ret = rawdb_exec_cell(cc, db);
                if (ret != MDB_ERR_NONE) {
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

static void rawdb_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct rawdb_entry *e = (struct rawdb_entry*)entry;
	int ret = REP_OK;
	
	mdb_conn *dbaccess = e->dbaccess;
	mdb_conn *dbstat = e->dbstat;
	struct cache *cd = e->cd;
	struct rawdb_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, ret);
	case REQ_CMD_ACCESS:
		ret = rawdb_cmd_updatedb(q, dbaccess);
		break;
	case REQ_CMD_STAT:
		ret = rawdb_cmd_updatedb(q, dbstat);
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
		mtc_err("process %u failed %d", q->operation, ret);
	}
	if (q->req->flags & FLAGS_SYNC) {
		reply_trigger(q, ret);
	}

#if 0
	if (st->proc_fai >= 1000 && st->proc_fai % 200 == 0) {
		mtc_err("proc_fai %lu", st->proc_fai);
		SMS_ALARM("db proc_fai %lu", st->proc_fai);
	} else if (st->proc_fai >= 100 && st->proc_fai % 20 == 0) {
		mtc_err("proc_fai %lu", st->proc_fai);
		SMS_ALARM("db proc_fai %lu", st->proc_fai);
	}

	if (st->msg_unrec == 1000 || st->msg_unrec == 1500) {
		SMS_ALARM("db msg_unrec %lu", st->msg_unrec);
	}
#endif
}

static void rawdb_stop_driver(struct event_entry *entry)
{
	struct rawdb_entry *e = (struct rawdb_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	mdb_destroy(e->dbaccess);
	mdb_destroy(e->dbstat);
	cache_free(e->cd);
}



static struct event_entry* rawdb_init_driver(void)
{
	struct rawdb_entry *e = calloc(1, sizeof(struct rawdb_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = rawdb_process_driver;
	e->base.stop_driver = rawdb_stop_driver;

	if (mdb_init(&e->dbaccess, hdf_get_value(g_cfg, CONFIG_PATH".access.dbsn", NULL)) != RET_RBTOP_OK) {
		wlog("init %s failure %s\n", PLUGIN_NAME, mdb_get_errmsg(e->dbaccess));
		goto error;
	}
	
	if (mdb_init(&e->dbstat, hdf_get_value(g_cfg, CONFIG_PATH".stat.dbsn", NULL)) != RET_RBTOP_OK) {
		wlog("init %s failure %s\n", PLUGIN_NAME, mdb_get_errmsg(e->dbstat));
		goto error;
	}
	
	e->cd = cache_create(hdf_get_int_value(g_cfg, CONFIG_PATH".numobjs", 1024), 0);
	if (e->cd == NULL) {
		wlog("init cache failure");
		goto error;
	}
	
	return (struct event_entry*)e;
	
error:
	if (e->base.name) free(e->base.name);
	if (e->dbaccess) mdb_destroy(e->dbaccess);
	if (e->dbstat) mdb_destroy(e->dbstat);
	if (e->cd) cache_free(e->cd);
	free(e);
	return NULL;
}

struct event_driver rawdb_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = rawdb_init_driver,
};
