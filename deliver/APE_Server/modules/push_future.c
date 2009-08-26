static void get_user_info(char *uin, USERS *user, acetables *g_ape)
{
	HTBL *ulist;
	extend *ext;
	struct data_cell *pc, *cc;
	uint32_t errcode;
	int ret;
	mevent_t *evt;

	evt = mevent_init();
	if (evt == NULL) {
		wlog_err("init event error");
		return;
	}

	mevent_add_tcp_server(evt, READ_CONF("uic_ip"), atoi(READ_CONF("uic_port")));
	mevent_chose_plugin(evt, "uic", 1001, FLAGS_SYNC);
	mevent_add_str(evt, NULL, "uin", uin);
	
	ret = mevent_trigger(evt, &errcode);
	if (ret != REP_OK) {
		wlog_err("get friend for user %s failure %d %lu", uin, ret, errcode);
		goto get_msgset;
	}
	
	pc = data_cell_search(evt->rcvdata, false, DATA_TYPE_ARRAY, "friend");
	if (pc != NULL) {
		ext = add_property(&user->properties, "friend", hashtbl_init(),
						   EXTEND_HTBL, EXTEND_ISPRIVATE);
		ulist = (HTBL*)ext->val;
		data_cell_array_iterate(pc, cc) {
			wlog_dbg("add friend %s", cc->v.sval.val);
			hashtbl_append(ulist, (char*)cc->v.sval.val, strdup((char*)cc->v.sval.val));
		}
	}

 get_msgset:
	mevent_chose_plugin(evt, "uic", 1005, FLAGS_SYNC);
	mevent_add_str(evt, NULL, "uin", uin);
	ret = mevent_trigger(evt, &errcode);
	if (ret != REP_OK) {
		wlog_err("get setting for user %s failure %d %lu", uin, ret, errcode);
		goto done;
	}
	
	pc = data_cell_search(evt->rcvdata, false, DATA_TYPE_ARRAY, "incept");
	if (pc != NULL) {
		ext = add_property(&user->properties, "incept", hashtbl_init(),
						   EXTEND_HTBL, EXTEND_ISPRIVATE);
		ulist = (HTBL*)ext->val;

		data_cell_array_iterate(pc, cc) {
			wlog_dbg("add incpet %s", cc->v.sval.val);
			hashtbl_append(ulist, (char*)cc->v.sval.val, strdup((char*)cc->v.sval.val));
		}
	}

	pc = data_cell_search(evt->rcvdata, false, DATA_TYPE_STRING, "msgset");
	if (pc != NULL) {
		wlog_dbg("add msgset %s", pc->v.sval.val);
		add_property(&user->properties, "msgset", pc->v.sval.val,
					 EXTEND_STR, EXTEND_ISPRIVATE);
	}

 done:
	mevent_free(evt);
	return;
}
