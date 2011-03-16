#include "apev.h"
#include "main.h"

static HASH *utbl = NULL;
static HASH *evth = NULL;

UserEntry* user_new()
{
	UserEntry *r = calloc(1, sizeof(UserEntry));
	if (!r) mtc_err("memory failure");
	return r;
}

static NEOERR* ext_cmd_useron(struct queue_entry *q)
{
	char *srcx, *uin;

	REQ_GET_PARAM_STR(q->hdfrcv, "srcx", srcx);
	REQ_GET_PARAM_STR(q->hdfrcv, "uin", uin);
	
	UserEntry *u = (UserEntry*)hash_lookup(utbl, uin);
	if (!u) {
		u = user_new();
		hash_insert(utbl, (void*)strdup(uin), (void*)u);
	}
	u->online = true;
	if (!u->server) u->server = strdup(srcx);
	else if (strcmp(u->server, srcx)) {
		free(u->server);
		u->server = strdup(srcx);
	}

	/*
	 * notice other aped
	 */
	char *key = NULL;
	mevent_t *evt = (mevent_t*)hash_next(evth, (void**)&key);

	while (evt) {
		if (strcmp(key, srcx)) {
			hdf_copy(evt->hdfsnd, NULL, q->hdfrcv);
			MEVENT_TRIGGER_NRET(evt, uin, REQ_CMD_USERON, FLAGS_NONE);
		}
		
		evt = hash_next(evth, (void**)&key);
	}
	
	return STATUS_OK;
}

static NEOERR* ext_cmd_useroff(struct queue_entry *q)
{
	char *srcx, *uin;

	REQ_GET_PARAM_STR(q->hdfrcv, "srcx", srcx);
	REQ_GET_PARAM_STR(q->hdfrcv, "uin", uin);

	UserEntry *u = (UserEntry*)hash_lookup(utbl, uin);
	if (!u) {
		u = user_new();
		hash_insert(utbl, (void*)strdup(uin), (void*)u);
	}
	u->online = false;
	
	/*
	 * notice other aped
	 */
	char *key = NULL;
	mevent_t *evt = (mevent_t*)hash_next(evth, (void**)&key);

	while (evt) {
		if (strcmp(key, srcx)) {
			hdf_copy(evt->hdfsnd, NULL, q->hdfrcv);
			MEVENT_TRIGGER_NRET(evt, uin, REQ_CMD_USEROFF, FLAGS_NONE);
		}
		
		evt = hash_next(evth, (void**)&key);
	}
	
	return STATUS_OK;
}

static NEOERR* ext_cmd_msgsnd(struct queue_entry *q)
{
	char *srcuin, *dstuin, *msg;
	char *srcx;
	
	if (!q) return nerr_raise(NERR_ASSERT, "input error");

	REQ_GET_PARAM_STR(q->hdfrcv, "srcx", srcx);
	REQ_GET_PARAM_STR(q->hdfrcv, "srcuin", srcuin);
	REQ_GET_PARAM_STR(q->hdfrcv, "dstuin", dstuin);
	REQ_GET_PARAM_STR(q->hdfrcv, "msg", msg);

	/*
	 * update source user info
	 */
	UserEntry *u = (UserEntry*)hash_lookup(utbl, srcuin);
	if (!u) {
		u = user_new();
		hash_insert(utbl, (void*)strdup(srcuin), (void*)u);
	}
	u->online = true;
	if (!u->server) u->server = strdup(srcx);
	else if (strcmp(u->server, srcx)) {
		free(u->server);
		u->server = strdup(srcx);
	}

	/*
	 * send msg to destnation user
	 */
	u = (UserEntry*)hash_lookup(utbl, dstuin);
	if (!u || !u->online) {
		/* dstuin offline now */
		mtc_dbg("user %s offline", dstuin);
	} else {
		/* send to dstuin */
		mevent_t *evt = (mevent_t*)hash_lookup(evth, u->server);
		if (!evt) return nerr_raise(NERR_ASSERT, "can't found %s", u->server);

		hdf_copy(evt->hdfsnd, NULL, q->hdfrcv);
		MEVENT_TRIGGER(evt, srcx, REQ_CMD_MSGSND, FLAGS_NONE);
	}
	
	return STATUS_OK;
}

static NEOERR* ext_process_driver(struct event_entry *e, struct queue_entry *q)
{
	NEOERR *err;

	switch(q->operation) {
	case REQ_CMD_USERON:
		err = ext_cmd_useron(q);
		break;
	case REQ_CMD_USEROFF:
		err = ext_cmd_useroff(q);
		break;
	case REQ_CMD_MSGSND:
		err = ext_cmd_msgsnd(q);
		break;
	case REQ_CMD_MSGBRD:
		//err = ext_cmd_msgbrd(q);
		break;
	}

	return nerr_pass(err);
}

static NEOERR* ext_start_driver()
{
	NEOERR *err;
	
	err = hash_init(&utbl, hash_str_hash, hash_str_comp);
	if (err != STATUS_OK) return nerr_pass(err);

	err = hash_init(&evth, hash_str_hash, hash_str_comp);
	if (err != STATUS_OK) return nerr_pass(err);

	HDF *node = hdf_get_obj(g_cfg, "Aped");
	if (!node) return nerr_raise(NERR_ASSERT, "Aped config not found");

	mevent_t *evt;
	char *ename;
	node = hdf_obj_child(node);
	while (node != NULL) {
		ename = hdf_obj_value(node);
		evt = mevent_init_plugin(ename);
		if (evt) {
			mtc_dbg("event %s init ok", ename);
			hash_insert(evth, (void*)strdup(ename), (void*)evt);
		} else {
			mtc_err("event %s init failure", ename);
		}
		
		node = hdf_obj_next(node);
	}

	return STATUS_OK;
}

struct event_entry ext_entry = {
	.name = "ape_ext_v",
	.start_driver = ext_start_driver,
	.process_driver = ext_process_driver,
	.stop_driver = NULL,
	.next = NULL,
};
