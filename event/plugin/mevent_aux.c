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
static NEOERR* aux_cmd_cmtget(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	int count, offset;
	char *ids, *idsdump, tok[128];
	int type = -1, oid = -1;
	NEOERR *err;

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
					err = mdb_set_rows(q->hdfsnd, db, CMT_COL, tok, -1);
					if (err != STATUS_OK) return nerr_pass(err);
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
			err = mdb_set_rows(q->hdfsnd, db, CMT_COL, tok, -1);
			if (err != STATUS_OK) return nerr_pass(err);
			mcs_html_escape(hdf_get_child(q->hdfsnd, tok), "content");
		}
		
		CACHE_HDF(q->hdfsnd, CMT_CC_SEC, PREFIX_CMTAPP"%s_%d", idsdump, offset);
		free(idsdump);
	}

	return STATUS_OK;
}

static NEOERR* aux_cmd_cmtadd(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	int type, oid, pid;
	char *ip, *addr, *author, *content;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "ip", ip);
	REQ_GET_PARAM_STR(q->hdfrcv, "addr", addr);
	REQ_GET_PARAM_STR(q->hdfrcv, "author", author);
	REQ_GET_PARAM_STR(q->hdfrcv, "content", content);

	REQ_GET_PARAM_INT(q->hdfrcv, "type", type);
	REQ_GET_PARAM_INT(q->hdfrcv, "oid", oid);
	REQ_FETCH_PARAM_INT(q->hdfrcv, "pid", pid);

	MDB_EXEC(db, NULL, "INSERT INTO comment "
			 " (type, oid, pid, ip, addr, author, content) "
			 " VALUES ($1, $2, $3, $4::varchar(256), "
			 " $5::varchar(256), $6::varchar(256), $7)",
			 "iiissss", type, oid, pid, ip, addr, author, content);
	
	cache_delf(cd, PREFIX_CMTAPP"%d:%d_0", type, oid);
	
	return STATUS_OK;
}

static NEOERR* aux_cmd_cmtdel(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	int id;
	NEOERR *err;

	REQ_GET_PARAM_INT(q->hdfrcv, "id", id);

	MDB_EXEC(db, NULL, "UPDATE comment SET state=%d WHERE id=%d;",
			 NULL, CMT_ST_DEL, id);
	
	return STATUS_OK;
}

static NEOERR* aux_cmd_mailadd(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	STRING str; string_init(&str);
	char sum[LEN_MD5], *content;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "content", content);
	mutil_md5_str(content, sum);
	hdf_set_value(q->hdfrcv, "checksum", sum);

	err = mcs_build_incol(q->hdfrcv,
						  hdf_get_obj(g_cfg, CONFIG_PATH".InsertCol.email"),
						  &str);
	if (err != STATUS_OK) return nerr_pass(err);

	MDB_EXEC(db, NULL, "ISNERT INTO mail %s", NULL, str.buf);

	string_clear(&str);

	return STATUS_OK;
}

static void aux_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct aux_entry *e = (struct aux_entry*)entry;
	NEOERR *err;
	int ret;
	
	mdb_conn *db = e->db;
	struct cache *cd = e->cd;
	struct aux_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, err);
	case REQ_CMD_CMT_GET:
		err = aux_cmd_cmtget(q, cd, db);
		break;
	case REQ_CMD_CMT_ADD:
		err = aux_cmd_cmtadd(q, cd, db);
		break;
	case REQ_CMD_CMT_DEL:
		err = aux_cmd_cmtdel(q, cd, db);
		break;
	case REQ_CMD_MAIL_ADD:
		err = aux_cmd_mailadd(q, cd, db);
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
	ret = neede ? err->error : REP_OK;
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
	NEOERR *err;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = aux_process_driver;
	e->base.stop_driver = aux_stop_driver;

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

struct event_driver aux_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = aux_init_driver,
};
