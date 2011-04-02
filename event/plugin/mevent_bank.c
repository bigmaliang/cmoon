#include "mevent_plugin.h"
#include "mevent_bank.h"

#define PLUGIN_NAME	"bank"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct bank_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct bank_entry {
	struct event_entry base;
	mdb_conn *db;
	struct cache *cd;
	struct bank_stats st;
};

#define BANKINFO_COL "aid, aname, msum, mpre, "		\
	" to_char(intime, 'YYYY-MM-DD') as intime, "	\
	" to_char(uptime, 'YYYY-MM-DD') as uptime "

#define BILL_COL "id, aid, aname, btype, fee, remark, " \
	" to_char(intime, 'YYYY-MM-DD HH:mm:SS') as intime "


static NEOERR* bank_cmd_info(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	char *aname;
	int aid;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	aid = hash_string(aname);

	if (cache_getf(cd, &val, &vsize, PREFIX_BANK"%d", aid)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		MDB_QUERY_RAW(db, "bank", BANKINFO_COL, "aid=%d", NULL, aid);
		err = mdb_set_row(q->hdfsnd, db, BANKINFO_COL, NULL);
		if (nerr_handle(&err, NERR_NOT_FOUND))
			return nerr_raise(REP_ERR_BANK_NEEDUP, "%s hasn't pay", aname);
		if (err != STATUS_OK) return nerr_pass(err);
		
		CACHE_HDF(q->hdfsnd, BILL_CC_SEC, PREFIX_BANK"%d", aid);
	}
	
	return STATUS_OK;
}

static NEOERR* bank_cmd_addbill(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	char *aname, *account, remark[256];
	int aid, type, fee, mm;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	REQ_GET_PARAM_INT(q->hdfrcv, "btype", type);
	REQ_GET_PARAM_INT(q->hdfrcv, "fee", fee);
	aid = hash_string(aname);

	err = mdb_begin(db);
	if (err != STATUS_OK) return nerr_pass(err);

	switch (type) {
	case BANK_OP_PRECHARGE:
		snprintf(remark, sizeof(remark), "user apply charge");

		MDB_EXEC_TS(db, NULL, "SELECT merge_bank_pre($1::varchar(256), $2, $3)",
					"sii", aname, aid, fee);
		
		break;
	case BANK_OP_CFMCHARGE:
		snprintf(remark, sizeof(remark), "bomdoo confirm charge");

		err = bank_cmd_info(q, cd, db);
		if (err != STATUS_OK) goto finish;
		
		if (hdf_get_int_value(q->hdfsnd, "mpre", 0) < fee) {
			err = nerr_raise(REP_ERR_BANK_NCHARGE, "%s mpre < %d", aname, fee);
			goto finish;
		}

		MDB_EXEC_TS(db, NULL, "UPDATE bank SET mpre = mpre-%d, msum = msum+%d WHERE "
					" aid=%d;",	NULL, fee, fee, aid);
		break;
	case BANK_OP_ADDACCOUNT:
		REQ_GET_PARAM_STR(q->hdfrcv, "account", account);
		snprintf(remark, sizeof(remark), "pay for %s open", account);

		err = bank_cmd_info(q, cd, db);
		if (err != STATUS_OK) goto finish;

		mm = hdf_get_int_value(q->hdfsnd, "msum", 0);
		if (mm < fee) {
			mdb_finish(db);
			mm = hdf_get_int_value(q->hdfsnd, "mpre", 0);
			if (mm > fee)
				err = nerr_raise(REP_ERR_BANK_NEEDWT, "%s need wait", aname);
			err = nerr_raise(REP_ERR_BANK_NEEDUP, "%s %d < %d", aname, mm, fee);
			goto finish;
		}

		MDB_EXEC_TS(db, NULL, "UPDATE bank SET msum = msum-%d WHERE aid=%d;",
					NULL, fee, aid);
		break;
	case BANK_OP_ROLLBACK:
		REQ_GET_PARAM_STR(q->hdfrcv, "remark", account);
		strncpy(remark, account, sizeof(remark));
		
		err = bank_cmd_info(q, cd, db);
		if (err != STATUS_OK) goto finish;

		MDB_EXEC_TS(db, NULL, "UPDATE bank SET msum = msum-%d WHERE aid=%d;",
					NULL, fee, aid);
		break;
	}

	MDB_EXEC_TS(db, NULL, "INSERT INTO bill (aid, aname, btype, fee, remark) "
				" VALUES ($1, $2::varchar(256), $3, $4, $5::varchar(256))",
				"isiis", aid, aname, type, fee, remark);

finish:
	if (err != STATUS_OK) mdb_finish(db);
	else err = mdb_finish(db);

	cache_delf(cd, PREFIX_BANK"%d", aid);

	STRING str; string_init(&str);
	hdf_set_int_value(q->hdfrcv, "aid", aid);
	mcs_build_querycond(q->hdfrcv,
						hdf_get_obj(g_cfg, CONFIG_PATH".QueryCond.bill"),
						&str, NULL);
	cache_delf(cd, PREFIX_BILL"%s_0", str.buf);
	string_clear(&str);
	
	return nerr_pass(err);
}

static NEOERR* bank_cmd_getbill(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	STRING str; string_init(&str);
	int aid, count, offset;
	char *aname;
	NEOERR *err;

	REQ_GET_PARAM_STR(q->hdfrcv, "aname", aname);
	aid = hash_string(aname);
	hdf_set_int_value(q->hdfrcv, "aid", aid);
	
	err = mcs_build_querycond(q->hdfrcv,
							  hdf_get_obj(g_cfg, CONFIG_PATH".QueryCond.bill"),
							  &str, NULL);
	if (err != STATUS_OK) return nerr_pass(err);

	mmisc_pagediv(q->hdfrcv, NULL, &count, &offset, NULL, q->hdfsnd);

	if (cache_getf(cd, &val, &vsize, PREFIX_BILL"%s_%d", str.buf, offset)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		MMISC_PAGEDIV_SET_N(q->hdfsnd, db, "bill", "%s", NULL, str.buf);

		MDB_QUERY_RAW(db, "bill", BILL_COL, "%s ORDER BY id DESC LIMIT %d OFFSET %d",
					  NULL, str.buf, count, offset);
		err = mdb_set_rows(q->hdfsnd, db, BILL_COL, "bills", 0);
		if (err != STATUS_OK) return nerr_pass(err);

		CACHE_HDF(q->hdfsnd, BILL_CC_SEC, PREFIX_BILL"%s_%d", str.buf, offset);
	}

	string_clear(&str);
	
	return STATUS_OK;
}

static void bank_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct bank_entry *e = (struct bank_entry*)entry;
	NEOERR *err;
	int ret;
	
	mdb_conn *db = e->db;
	struct cache *cd = e->cd;
	struct bank_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, e->cd, err);
	case REQ_CMD_BANK_INFO:
		err = bank_cmd_info(q, cd, db);
		break;
	case REQ_CMD_BANK_ADDBILL:
		err = bank_cmd_addbill(q, cd, db);
		break;
	case REQ_CMD_BANK_GETBILL:
		err = bank_cmd_getbill(q, cd, db);
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

static void bank_stop_driver(struct event_entry *entry)
{
	struct bank_entry *e = (struct bank_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	mdb_destroy(e->db);
	cache_free(e->cd);
}



static struct event_entry* bank_init_driver(void)
{
	struct bank_entry *e = calloc(1, sizeof(struct bank_entry));
	if (e == NULL) return NULL;
	NEOERR *err;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = bank_process_driver;
	e->base.stop_driver = bank_stop_driver;

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

struct event_driver bank_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = bank_init_driver,
};
