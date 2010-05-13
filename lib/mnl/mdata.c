#include "mheads.h"

#ifdef USE_EVENT

static mevent_t *l_evt;
static bool l_inited = false;

static int mdata_init()
{
	if (l_inited) return RET_RBTOP_OK;
	
	l_evt = mevent_init();
	if (l_evt == NULL) {
		mtc_err("mevent_init faiure");
		return RET_RBTOP_MEMALLOCE;
	}

	HDF *node = hdf_get_obj(g_cfg, PRE_CFG_EVT);
	if (node != NULL) node = hdf_obj_child(node);

	while (node != NULL) {
		mevent_add_udp_server(l_evt, hdf_get_value(node, "ip", "127.0.0.1"),
							  hdf_get_int_value(node, "port", 26010));

		node = hdf_obj_next(node);
	}

	l_inited = true;
	return RET_RBTOP_OK;
	
}

int mdata_exec(char *plugin, int *afrow, unsigned short flag,
			   const char *sql_fmt, const char *fmt, ...)
{
	uint32_t errcode = 0;
	char *sql;
	int ret;

	ret = mdata_init();
	if (ret != RET_RBTOP_OK) return ret;

	va_list ap;
	va_start(ap, sql_fmt);
	ret = mutil_expand_strvf(&sql, sql_fmt, fmt, ap);
	va_end(ap);
	if (ret != RET_RBTOP_OK) {
		mtc_err("expand ~ %s %s failure", sql_fmt, fmt);
		return ret;
	}

    /* TODO change to mevent_inint_plugin */
	mevent_chose_plugin(l_evt, plugin, REQ_CMD_SET, flag);
	mevent_add_str(l_evt, NULL, "sqls", sql);
	
	free(sql);
	
	ret = mevent_trigger(l_evt, &errcode);
	if (ret != REP_OK) {
		mtc_err("trig %s failure %d %d", ret, errcode);
		return ret;
	}
	
	return RET_RBTOP_OK;
}
#else
int mdata_exec(char *plugin, int *afrow, unsigned short flag,
			   const char *sql_fmt, const char *fmt, ...)
{
	return RET_RBTOP_OK;
}
#endif
