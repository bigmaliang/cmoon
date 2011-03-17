#include "apev.h"
#include "main.h"

static HASH *utbl = NULL;
static HASH *stbl = NULL;

static NEOERR* ext_cmd_useron(struct queue_entry *q)
{
	char *srcx, *uin;

	REQ_GET_PARAM_STR(q->hdfrcv, "srcx", srcx);
	REQ_GET_PARAM_STR(q->hdfrcv, "uin", uin);

	mtc_dbg("server %s's user %s on", srcx, uin);

	UserEntry *u = (UserEntry*)hash_lookup(utbl, uin);
	if (!u) {
		u = user_new();
		hash_insert(utbl, (void*)strdup(uin), (void*)u);
	}
	u->online = true;
	
	char *key = NULL;
	SnakeEntry *s = (SnakeEntry*)hash_next(stbl, (void**)&key);

	while (s) {
		if (strcmp(key, srcx)) {
			/*
			 * notice other aped
			 */
			hdf_copy(s->evt->hdfsnd, NULL, q->hdfrcv);
			MEVENT_TRIGGER_NRET(s->evt, uin, REQ_CMD_USERON, FLAGS_NONE);
		} else {
			u->server = s->name;
			s->num_online++;
		}
		
		s = hash_next(stbl, (void**)&key);
	}
	
	return STATUS_OK;
}

static NEOERR* ext_cmd_useroff(struct queue_entry *q)
{
	char *srcx, *uin;

	REQ_GET_PARAM_STR(q->hdfrcv, "srcx", srcx);
	REQ_GET_PARAM_STR(q->hdfrcv, "uin", uin);

	mtc_dbg("server %s's user %s off", srcx, uin);
	
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
	SnakeEntry *s = (SnakeEntry*)hash_next(stbl, (void**)&key);

	while (s) {
		if (strcmp(key, srcx)) {
			hdf_copy(s->evt->hdfsnd, NULL, q->hdfrcv);
			MEVENT_TRIGGER_NRET(s->evt, uin, REQ_CMD_USEROFF, FLAGS_NONE);
		} else {
			if (s->num_online > 0) s->num_online--;
		}
		
		s = hash_next(stbl, (void**)&key);
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

	mtc_dbg("server %s's user %s say %s to %s",
			srcx, srcuin, msg, dstuin);
	
	/*
	 * update source user info
	 */
	UserEntry *u = (UserEntry*)hash_lookup(utbl, srcuin);
	if (!u) {
		u = user_new();
		hash_insert(utbl, (void*)strdup(srcuin), (void*)u);
	}
	u->online = true;

	SnakeEntry *s = (SnakeEntry*)hash_lookup(stbl, srcx);
	if (!s) u->server = strdup(srcx);
	else u->server = s->name;

	/*
	 * send msg to destnation user
	 */
	u = (UserEntry*)hash_lookup(utbl, dstuin);
	if (!u || !u->online) {
		/*
		 * dstuin offline now
		 * TODO offline message
		 */
		mtc_dbg("user %s offline", dstuin);
	} else {
		/* send to dstuin */
		SnakeEntry *s = (SnakeEntry*)hash_lookup(stbl, u->server);
		if (!s) return nerr_raise(NERR_ASSERT, "can't found %s", u->server);

		hdf_copy(s->evt->hdfsnd, NULL, q->hdfrcv);
		MEVENT_TRIGGER(s->evt, srcx, REQ_CMD_MSGSND, FLAGS_NONE);
	}
	
	return STATUS_OK;
}

static NEOERR* ext_cmd_state(struct queue_entry *q)
{
	unsigned int ttnum = 0;
	char *key = NULL;
	SnakeEntry *s = (SnakeEntry*)hash_next(stbl, (void**)&key);

	while (s) {
		ttnum += s->num_online;

		s = hash_next(stbl, (void**)&key);
	}
	hdf_set_int_value(q->hdfsnd, "total online", ttnum);

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
	case REQ_CMD_STATE:
		err = ext_cmd_state(q);
		break;
	default:
		err = nerr_raise(NERR_ASSERT, "unknown command %u", q->operation);
		break;
	}

	return nerr_pass(err);
}

static NEOERR* ext_start_driver()
{
	NEOERR *err;
	
	err = hash_init(&utbl, hash_str_hash, hash_str_comp);
	if (err != STATUS_OK) return nerr_pass(err);

	err = hash_init(&stbl, hash_str_hash, hash_str_comp);
	if (err != STATUS_OK) return nerr_pass(err);

	HDF *node = hdf_get_obj(g_cfg, "Aped");
	if (!node) return nerr_raise(NERR_ASSERT, "Aped config not found");

	char *ename;
	node = hdf_obj_child(node);
	while (node != NULL) {
		ename = hdf_obj_value(node);
		SnakeEntry *s = snake_new(ename);
		if (s) {
			mtc_dbg("event %s init ok", ename);
			hash_insert(stbl, (void*)strdup(ename), (void*)s);
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
