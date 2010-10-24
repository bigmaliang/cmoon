#include "mevent_plugin.h"
#include "mevent_aux.h"

#define PLUGIN_NAME	"aux"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct aux_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct aux_entry {
	struct event_entry base;
	mdb_conn *db;
	struct cache *cd;
	struct aux_stats st;
};

#define CMT_COL " id, type, oid, pid, ip, addr, author, content, " \
	" to_char(intime, 'YYYY-MM-DD HH24:MI:SS') as intime, "		   \
	" to_char(uptime, 'YYYY-MM-DD HH24:MI:SS') as uptime "

/*
 * ids=0:1
 * { "0": { "1": { "ntt": "0", "nst": "0" } }, "success": "1" }
 *
 * ids=0:1,0:20
 * { "0": { "1": { "ntt": "0", "nst": "0" }, "20": { "ntt": "0", "nst": "0" } }, "success": "1" }
 *
 * ids=0:1,1:20
 * { "0": { "1": { "ntt": "0", "nst": "0" } }, "1": { "20": { "ntt": "0", "nst": "0" } }, "success": "1" } 
 */
static int aux_cmd_cmtget(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	int count, offset;
	char *ids, *idsdump, tok[128];
	int type = -1, oid = -1;

	REQ_GET_PARAM_STR(q->hdfrcv, "ids", ids);

	mmisc_pagediv_get(q->hdfrcv, NULL, &count, &offset);

	if (cache_getf(cd, &val, &vsize, PREFIX_CMTAPP"%s_%d", ids, offset)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		idsdump = strdup(ids);
		char *p = ids;
		while (*p) {
			if (*p == ':') {
				*p = '\0';
				type = atoi(ids);
				ids = p+1;
			}
			if (*p == ',') {
				*p = '\0';
				oid = atoi(ids);
				if (type >= 0 && oid >= 0) {
					sprintf(tok, "%d.%d", type, oid);
					MMISC_PAGEDIV_SET(q->hdfsnd, tok, offset, db, "comment",
									  "type=%d AND state=%d AND oid=%d",
									  NULL, type, CMT_ST_NORMAL, oid);
					MDB_QUERY_RAW(db, "comment", CMT_COL,
								  "type=%d AND state=%d AND oid=%d "
								  " ORDER BY intime DESC LIMIT %d OFFSET %d",
								  NULL, type, CMT_ST_NORMAL, oid, count, offset);
					sprintf(tok, "%d.%d.cmts", type, oid);
					mdb_set_rows(q->hdfsnd, db, CMT_COL, tok, -1);
					mcs_html_escape(hdf_get_child(q->hdfsnd, tok), "content");
					type = oid = -1;
				}
				ids = p+1;
			}
			p++;
		}
		oid = atoi(ids);
		if (type >= 0 && oid >=0) {
			sprintf(tok, "%d.%d", type, oid);
			MMISC_PAGEDIV_SET(q->hdfsnd, tok, offset, db, "comment",
							  "type=%d AND state=%d AND oid=%d",
							  NULL, type, CMT_ST_NORMAL, oid);
			MDB_QUERY_RAW(db, "comment", CMT_COL,
						  "type=%d AND state=%d AND oid=%d "
						  " ORDER BY intime DESC LIMIT %d OFFSET %d",
						  NULL, type, CMT_ST_NORMAL, oid, count, offset);
			sprintf(tok, "%d.%d.cmts", type, oid);
			mdb_set_rows(q->hdfsnd, db, CMT_COL, tok, -1);
			mcs_html_escape(hdf_get_child(q->hdfsnd, tok), "content");
		}
		
		CACHE_HDF(q->hdfsnd, CMT_CC_SEC, PREFIX_CMTAPP"%s_%d", idsdump, offset);
		free(idsdump);
	}

	return REP_OK;
}

static int aux_cmd_cmtadd(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	int type, oid, pid;
	char *ip, *addr, *author, *content;

	REQ_GET_PARAM_STR(q->hdfrcv, "ip", ip);
	REQ_GET_PARAM_STR(q->hdfrcv, "addr", addr);
	REQ_GET_PARAM_STR(q->hdfrcv, "author", author);
	REQ_GET_PARAM_STR(q->hdfrcv, "content", content);

	REQ_GET_PARAM_INT(q->hdfrcv, "type", type);
	REQ_GET_PARAM_INT(q->hdfrcv, "oid", oid);
	REQ_FETCH_PARAM_INT(q->hdfrcv, "pid", pid);

	MDB_EXEC_EVT(db, NULL, "INSERT INTO comment "
				 " (type, oid, pid, ip, addr, author, content) "
				 " VALUES ($1, $2, $3, $4, $5, $6, $7)",
				 "iiissss", type, oid, pid, ip, addr, author, content);

	cache_delf(cd, PREFIX_CMTAPP"%d:%d_0", type, oid);

	return REP_OK;
}

static int aux_cmd_cmtdel(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	int id;

	REQ_GET_PARAM_INT(q->hdfrcv, "id", id);

	MDB_EXEC_EVT(db, NULL, "UPDATE comment SET state=%d WHERE id=%d;",
				 NULL, CMT_ST_DEL, id);
	
	return REP_OK;
}

static void aux_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct aux_entry *e = (struct aux_entry*)entry;
	int ret = REP_OK;
	
	mdb_conn *db = e->db;
	struct cache *cd = e->cd;
	struct aux_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, ret);
	case REQ_CMD_CMT_GET:
		ret = aux_cmd_cmtget(q, cd, db);
		break;
	case REQ_CMD_CMT_ADD:
		ret = aux_cmd_cmtadd(q, cd, db);
		break;
	case REQ_CMD_CMT_DEL:
		ret = aux_cmd_cmtdel(q, cd, db);
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

static void aux_stop_driver(struct event_entry *entry)
{
	struct aux_entry *e = (struct aux_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	mdb_destroy(e->db);
	cache_free(e->cd);
}



static struct event_entry* aux_init_driver(void)
{
	struct aux_entry *e = calloc(1, sizeof(struct aux_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = aux_process_driver;
	e->base.stop_driver = aux_stop_driver;

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

struct event_driver aux_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = aux_init_driver,
};
