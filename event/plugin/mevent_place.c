#include "mevent_plugin.h"
#include "mevent_place.h"

#define PLUGIN_NAME	"place"
#define CONFIG_PATH	PRE_PLUGIN"."PLUGIN_NAME

static unsigned char *ips = NULL;
static unsigned int ipbgn, ipend;
static iconv_t cv = NULL;

struct place_stats {
	unsigned long msg_total;
	unsigned long msg_unrec;
	unsigned long msg_badparam;
	unsigned long msg_stats;

	unsigned long proc_suc;
	unsigned long proc_fai;
};

struct place_entry {
	struct event_entry base;
	mdb_conn *db;
	struct cache *cd;
	struct place_stats st;
};

static char* gb2utf8(char *s)
{
	if (!s || !*s) return NULL;

	if (!cv || cv == (iconv_t)-1) cv = iconv_open("UTF-8", "GB2312");
	if (cv == (iconv_t)-1) {
		mtc_err("init conv error %s", strerror(errno));
		return NULL;
	}

	size_t len = strlen(s), ulen;
	ulen = len*2;
	char *utf8 = calloc(1, ulen);
	char *us = utf8;
	
	if (iconv (cv, &s, &len, &utf8, &ulen) == -1) {
		mtc_err("conv error");
		free(us);
		return NULL;
	}
	
	//iconv_close(cv);
	return us;
}

static unsigned int b2int(unsigned char *p, int count)
{
	int i;
	unsigned int ret;

	if(count < 1 || count > 4) 
		return 0;
	
	ret = p[0];
	for (i = 0; i < count; i++)
		ret |= ((unsigned int)p[i])<<(8*i);
	
	return ret;
}

static unsigned char* readarea(int offset)
{
	if (!ips) return NULL;
	
	unsigned char *p = ips + offset;
	unsigned char mode = *p;
	
	if (mode == REDIRECT_MODE_1 || mode == REDIRECT_MODE_2) {
		offset = b2int(p+1, 3);
		if (offset == 0) {
			return NULL;
		}
	}
	return ips + offset;
}

static void ip_place(int offset, char **c, char **a)
{
	if (!ips) return;
	
	unsigned char *p = ips + offset + 4;
	unsigned char mode = *p;

	if (mode == REDIRECT_MODE_1) {
		offset = b2int(p+1, 3);
		p = ips + offset;
		mode = *p;
		if (mode == REDIRECT_MODE_2) {
			*c = (char*)(ips + b2int(p+1, 3));
			*a = (char*)readarea(offset+4);
		} else {
			*c = (char*)(ips + offset);
			*a = (char*)readarea(offset + strlen(*c) + 1);
		}
	} else if (mode == REDIRECT_MODE_2) {
		offset = b2int(p+1, 3);
		*c = (char*)(ips + offset);
		*a = (char*)readarea(offset+4+4);
	} else {
		*c = (char*)(ips + offset + 4);
		*a = (char*)(readarea(offset + 4 + strlen(*c) + 1));
	}
	*c = gb2utf8(*c);
	*a = gb2utf8(*a);
}

static int ip_offset(unsigned int ip)
{
	unsigned int ipb, ipe;
	unsigned int M, L, R, record_count;

	record_count = (ipend - ipbgn)/7+1;
	/* search for right range */
	L = 0;
	R = record_count - 1;
	while (L < R-1) {
		M = (L + R) / 2;
		ipb = b2int(ips + ipbgn + M*7, 4);
		ipe = b2int(ips + b2int(ips + ipbgn + M*7 + 4, 3), 4);

		if (ip == ipb) {
			L = M;
			break;
		}
		if (ip > ipb)
			L = M;
		else
			R = M;
	}

	ipb = b2int(ips + ipbgn + L*7, 4);
	ipe = b2int(ips + b2int(ips + ipbgn + L*7 + 4, 3), 4);

	/* version infomation, the last item */
	if((ip & 0xffffff00) == 0xffffff00) {
		ipb = b2int(ips + ipbgn + R*7, 4);
		ipe = b2int(ips + b2int(ips + ipbgn + R*7 + 4, 3), 4);
	}
	
	if (ipb <= ip && ip <= ipe)
		return b2int(ips + ipbgn + L*7 + 4, 3);
	else
		return -1;
}

static bool ip2place(HDF *hdf, char *ip, char *key)
{
	if (!hdf || !ip) return false;
	
	unsigned int dip, offset;
	char *c, *a;

	dip= inet_addr(ip);
	
	if (dip != INADDR_NONE) {
		dip = ntohl(dip);
		offset = ip_offset(dip);
		if (offset >= 0) {
			ip_place(offset, &c, &a);
			if (c && a) {
				if (key) {
					hdf_set_valuef(hdf, "%s.ip=%s", key, ip);
					hdf_set_valuef(hdf, "%s.c=%s", key, c);
					hdf_set_valuef(hdf, "%s.a=%s", key, a);
				} else {
					hdf_set_valuef(hdf, "ip=%s", ip);
					hdf_set_valuef(hdf, "c=%s", c);
					hdf_set_valuef(hdf, "a=%s", a);
				}
				free(c); c = NULL;
				free(a); a = NULL;
				return true;
			}
		}
	}
	return false;
}

static NEOERR* place_cmd_get(struct queue_entry *q, struct cache *cd, mdb_conn *db)
{
	unsigned char *val = NULL; size_t vsize = 0;
	int count = 0;
	char *ip, tok[64];
	
	REQ_GET_PARAM_STR(q->hdfrcv, "ip", ip);
	
	if (cache_getf(cd, &val, &vsize, PREFIX_PLACE"%s", ip)) {
		unpack_hdf(val, vsize, &q->hdfsnd);
	} else {
		char *s = strdup(ip);
		char *dupip = s, *p = s;
		while (*p != '\0') {
			if (*p == ',') {
				*p = '\0';
				sprintf(tok, "%d", count);
				ip2place(q->hdfsnd, s, tok);
				count++;
				s = p+1;
			}
			p++;
		}
		sprintf(tok, "%d", count);
		ip2place(q->hdfsnd, s, tok);
		free(dupip);

		CACHE_HDF(q->hdfsnd, 0, PREFIX_PLACE"%s", ip);
	}
	
	return STATUS_OK;
}

static void place_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct place_entry *e = (struct place_entry*)entry;
	NEOERR *err;
	int ret;
	
	mdb_conn *db = e->db;
	struct cache *cd = e->cd;
	struct place_stats *st = &(e->st);

	st->msg_total++;
	
	mtc_dbg("process cmd %u", q->operation);
	switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, cd, err);
	case REQ_CMD_PLACEGET:
		err = place_cmd_get(q, cd, db);
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

static void place_stop_driver(struct event_entry *entry)
{
	struct place_entry *e = (struct place_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	mdb_destroy(e->db);
	cache_free(e->cd);
	if (ips) {
		free(ips);
		ips = NULL;
	}
}



static struct event_entry* place_init_driver(void)
{
	struct place_entry *e = calloc(1, sizeof(struct place_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = place_process_driver;
	e->base.stop_driver = place_stop_driver;

	e->cd = cache_create(hdf_get_int_value(g_cfg, CONFIG_PATH".numobjs", 1024), 0);
	if (e->cd == NULL) {
		wlog("init cache failure");
		goto error;
	}

	char *f = hdf_get_value(g_cfg, CONFIG_PATH".ipfile", "QQWry.Dat");
	NEOERR *err = ne_load_file(f, (char**)&ips);
	JUMP_NOK(err, error);

	ipbgn = b2int(ips, 4);
	ipend = b2int(ips+4, 4);
	if (ipbgn < 0 || ipend < 0) {
		wlog("%s format error", f);
		goto error;
	}
	
	return (struct event_entry*)e;
	
error:
	if (e->base.name) free(e->base.name);
	if (e->db) mdb_destroy(e->db);
	if (e->cd) cache_free(e->cd);
	if (ips) free(ips);
	free(e);
	return NULL;
}

struct event_driver place_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = place_init_driver,
};
