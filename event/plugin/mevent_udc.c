#include "mevent_plugin.h"
#include "mevent_udc.h"

#define PLUGIN_NAME	"udc"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

struct udc_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct udc_entry {
	struct event_entry base;
	FILE *logf;
	fdb_t *db;
	struct cache *cd;
	struct udc_stats st;
};

/*
 * input : uin(UINT)
 * return: NORMAL
 * reply : ["bonus": 1234] OR ["bonus": 0]
 */
static int udc_cmd_getbonus(struct queue_entry *q, struct cache *cd,
							fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	unsigned char *val = NULL;
	size_t vsize = 0;
	int uin, hit;
	
    REQ_GET_PARAM_U32(c, q, false, "uin", uin);
    
    reply_add_u32(q, NULL, "bonus", 0);
    hit = cache_getf(cd, &val, &vsize, PREFIX_BONUS"%d", uin);
    if (hit == 0) {
        snprintf(db->sql, sizeof(db->sql), "SELECT bonus FROM user_info "
                 " WHERE userid=%d;", uin);
        if (fdb_exec(db) != RET_DBOP_OK) {
            dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
            return REP_ERR_DB;
        }
        if (fdb_fetch_row(db) == RET_DBOP_OK) {
            reply_add_u32(q, NULL, "bonus", atoi(db->row[0]));
        }
        val = calloc(1, MAX_PACKET_LEN);
        if (val == NULL) {
            return REP_ERR_MEM;
        }
        vsize = pack_data_array(NULL, q->replydata, val,
                                MAX_PACKET_LEN - RESERVE_SIZE);
        if (vsize == 0) {
            free(val);
            return REP_ERR_PACK;
        }
        * (uint32_t *) (val+vsize) = htonl(DATA_TYPE_EOF);
        vsize += sizeof(uint32_t);
        cache_setf(cd, val, vsize, PREFIX_BONUS"%d", uin);
        free(val);
    } else {
        unpack_data("root", val, vsize, &q->replydata);
    }

	return REP_OK;
}

/*
 * input : uin(UINT) bonus(INT)
 * return: NORMAL
 * reply : ["bonus": 1234] OR ["bonus": 0]
 */
static int udc_cmd_setbonus(struct queue_entry *q, struct cache *cd,
							fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	int uin, bonus;

    REQ_GET_PARAM_U32(c, q, false, "uin", uin);
    REQ_GET_PARAM_U32(c, q, false, "bonus", bonus);

	snprintf(db->sql, sizeof(db->sql), "UPDATE user_info SET bonus=bonus+%d "
			 " WHERE userid=%d;", bonus, uin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}

	cache_delf(cd, PREFIX_BONUS"%d", uin);

	return udc_cmd_getbonus(q, cd, db, fp);
}

/*
 * input : uin(UINT)
 * return: NORMAL
 * reply : ["exp": 1234, "level": 1] OR ["exp": 0, "level": 1]
 */
static int udc_cmd_getexp(struct queue_entry *q, struct cache *cd,
						  fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	unsigned char *val = NULL;
	size_t vsize = 0;
	int uin, hit;
	
    REQ_GET_PARAM_U32(c, q, false, "uin", uin);

    reply_add_u32(q, NULL, "exp", 0);
    reply_add_u32(q, NULL, "level", 1);
    hit = cache_getf(cd, &val, &vsize, PREFIX_EXP"%d", uin);
    if (hit == 0) {
        snprintf(db->sql, sizeof(db->sql), "SELECT experience, level FROM "
                 " user_info WHERE userid=%d;", uin);
        if (fdb_exec(db) != RET_DBOP_OK) {
            dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
            return REP_ERR_DB;
        }
        if (fdb_fetch_row(db) == RET_DBOP_OK) {
            reply_add_u32(q, NULL, "exp", atoi(db->row[0]));
            reply_add_u32(q, NULL, "level", atoi(db->row[1]));
        }
        val = calloc(1, MAX_PACKET_LEN);
        if (val == NULL) {
            return REP_ERR_MEM;
        }
        vsize = pack_data_array(NULL, q->replydata, val,
                                MAX_PACKET_LEN - RESERVE_SIZE);
        if (vsize == 0) {
            free(val);
            return REP_ERR_PACK;
        }
        * (uint32_t *) (val+vsize) = htonl(DATA_TYPE_EOF);
        vsize += sizeof(uint32_t);
        cache_setf(cd, val, vsize, PREFIX_EXP"%d", uin);
        free(val);
    } else {
        unpack_data("root", val, vsize, &q->replydata);
    }

	return REP_OK;
}

/*
 * input : uin(UINT) exp(UINT)
 * return: NORMAL
 * reply : ["exp": 34, "level": 1]  OR ["exp": 1234, "level": 3, "levelup": 2]
 * OR ["exp": 0, "level": 1]
 */
static int udc_cmd_setexp(struct queue_entry *q, struct cache *cd,
						  fdb_t *db, FILE *fp)
{
	struct data_cell *c;
	int uin, exp;

    REQ_GET_PARAM_U32(c, q, false, "uin", uin);
    REQ_GET_PARAM_U32(c, q, false, "exp", exp);

	snprintf(db->sql, sizeof(db->sql), "UPDATE user_info SET "
			 " experience=experience+%d WHERE userid=%d;", exp, uin);
	if (fdb_exec(db) != RET_DBOP_OK) {
		dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
		return REP_ERR_DB;
	}

	cache_delf(cd, PREFIX_EXP"%d", uin);
    udc_cmd_getexp(q, cd, db, fp);

    exp = 0;
    int level = 1, uped = 0;
    char tok[64];
    REQ_FETCH_REPLY_U32(c, q, false, "exp", exp);
    REQ_FETCH_REPLY_U32(c, q, false, "level", level);

    sprintf(tok, CONFIG_PATH".exp_level.%d", level);
    while (exp >= hdf_get_int_value(g_cfg, tok, -1) &&
           hdf_get_int_value(g_cfg, tok, -1) != -1) {
        level = level + 1;
        snprintf(db->sql, sizeof(db->sql), "UPDATE user_info SET "
                 " level=%d WHERE userid=%d;", level, uin);
        if (fdb_exec(db) != RET_DBOP_OK) {
            dtc_err(fp, "exec %s failure %s", db->sql, fdb_error(db));
        }
        reply_add_u32(q, NULL, "levelup", ++uped);
        sprintf(tok, CONFIG_PATH".exp_level.%d", level);
    }
    
	cache_delf(cd, PREFIX_EXP"%d", uin);
	return REP_OK;
}

static void udc_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct udc_entry *e = (struct udc_entry*)entry;
	int ret = REP_OK;
	
	FILE *fp = e->logf;
	fdb_t *db = e->db;
	struct cache *cd = e->cd;
	struct udc_stats *st = &(e->st);

	st->msg_total++;
	
	dtc_dbg(fp, "process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, db, fp, ret);
	case REQ_CMD_GETBONUS:
		ret = udc_cmd_getbonus(q, cd, db, fp);
		break;
	case REQ_CMD_SETBONUS:
		ret = udc_cmd_setbonus(q, cd, db, fp);
		break;
	case REQ_CMD_GETEXP:
		ret = udc_cmd_getexp(q, cd, db, fp);
		break;
	case REQ_CMD_SETEXP:
		ret = udc_cmd_setexp(q, cd, db, fp);
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
}

static void udc_stop_driver(struct event_entry *entry)
{
	struct udc_entry *e = (struct udc_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	dtc_leave(e->logf);
	fdb_free(&e->db);
	cache_free(e->cd);
}



static struct event_entry* udc_init_driver(void)
{
	struct udc_entry *e = calloc(1, sizeof(struct udc_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = udc_process_driver;
	e->base.stop_driver = udc_stop_driver;

	e->logf = dtc_init(hdf_get_value(g_cfg, CONFIG_PATH".logfile",
									 TC_ROOT"plugin/udc"));
	if (e->logf == NULL) {
		wlog("open log file failure\n");
		goto error;
	}
	
	if (fdb_init_long(&e->db, hdf_get_value(g_cfg, CONFIG_PATH".ip", "127.0.0.1"),
					  hdf_get_value(g_cfg, CONFIG_PATH".user", "test"),
					  hdf_get_value(g_cfg, CONFIG_PATH".pass", "test"),
					  hdf_get_value(g_cfg, CONFIG_PATH".name", "user_info"),
					  (unsigned int)hdf_get_int_value(g_cfg, CONFIG_PATH".port", 0))
		!= RET_DBOP_OK) {
		wlog("init %s failure %s\n", PLUGIN_NAME, fdb_error(e->db));
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
	if (e->logf) dtc_leave(e->logf);
	if (e->db) fdb_free(&e->db);
	if (e->cd) cache_free(e->cd);
	free(e);
	return NULL;
}

struct event_driver udc_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = udc_init_driver,
};
