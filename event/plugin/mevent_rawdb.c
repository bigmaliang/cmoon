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

static int rawdb_exec_str(struct queue_entry *q, char *sql, mdb_conn *db)
{
	int ret = mdb_exec(db, NULL, sql, NULL);
	if (ret != MDB_ERR_NONE) {
		mtc_err("exec %s failure %s", sql, mdb_get_errmsg(db));
	} else {
		int id;
		if (mdb_get(db, "i", &id) == MDB_ERR_NONE) {
			hdf_set_int_value(q->hdfsnd, "id", id);
		}
/* TODO mysql and sqlite use the mdb_get_last_id() */
#if 0
		if (strncasecmp(sql, "insert", strlen("insert"))) {
			hdf_set_int_value(q->hdfsnd, "id", mdb_get_last_id(db, NULL));
		}
#endif
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
	HDF *node;
	char *sql;
    int ret;

	node = hdf_get_obj(q->hdfrcv, "sqls");
	if (node) {
		if (hdf_obj_child(node)) {
			node = hdf_obj_child(node);
			while (node) {
				sql = hdf_obj_value(node);
				mtc_dbg("exec %s ...\n", sql);
				ret = rawdb_exec_str(q, sql, db);
				if (ret != MDB_ERR_NONE) {
					if (!strcmp(hdf_obj_name(node), MASTER_EVT_NAME)) {
						return REP_ERR_DB;
					}
				}
				node = hdf_obj_next(node);
			}
		} else {
			sql = hdf_obj_value(node);
			mtc_dbg("exec %s ...\n", sql);
			ret = rawdb_exec_str(q, sql, db);
			if (ret != MDB_ERR_NONE) {
				return REP_ERR_DB;
			}
		}
	} else {
        mtc_err("excutable sqls not found\n");
        return REP_ERR_BADPARAM;
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

	char *dbsn = hdf_get_value(g_cfg, CONFIG_PATH".access.dbsn", NULL);
	if (mdb_init(&e->dbaccess, dbsn) != RET_RBTOP_OK) {
		wlog("init %s failure %s\n", dbsn, mdb_get_errmsg(e->dbaccess));
		goto error;
	} else {
		mtc_info("init %s ok", dbsn);
	}

	dbsn = hdf_get_value(g_cfg, CONFIG_PATH".stat.dbsn", NULL);
	if (mdb_init(&e->dbstat, dbsn) != RET_RBTOP_OK) {
		wlog("init %s failure %s\n", dbsn, mdb_get_errmsg(e->dbstat));
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
