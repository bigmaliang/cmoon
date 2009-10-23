#include "plugins.h"
#include "config.h"
#include <mysql/mysql.h>

#include "mevent.h"
#include "data.h"
#include "mevent_uic.h"

#define MODULE_NAME "push"

#define ERR_UIN_USED 	"[\n{\"raw\":\"ERR\",\"time\":null,\"datas\":" \
	"{\"code\":\"005\",\"value\":\"UIN_USED\"}}\n]\n"
#define ERR_BAD_UIN		"[\n{\"raw\":\"ERR\",\"time\":null,\"datas\":" \
	"{\"code\":\"006\",\"value\":\"BAD_UIN\"}}\n]\n"
#define ERR_MAKE_CHAN	"[\n{\"raw\":\"ERR\",\"time\":null,\"datas\":" \
	"{\"code\":\"007\",\"value\":\"ERR_MAKE_CHANNEL\"}}\n]\n"
#define ERR_REGIST_APP	"[\n{\"raw\":\"ERR\",\"time\":null,\"datas\":" \
	"{\"code\":\"008\",\"value\":\"ERR_REGIST_APP\"}}\n]\n"
#define ERR_UIN_NEXIST	"[\n{\"raw\":\"ERR\",\"time\":null,\"datas\":" \
	"{\"code\":\"009\",\"value\":\"ERR_UIN_NEXIST\"}}\n]\n"
#define ERR_USER_REFUSE	"[\n{\"raw\":\"ERR\",\"time\":null,\"datas\":" \
	"{\"code\":\"010\",\"value\":\"ERR_USER_REFUSE\"}}\n]\n"
#define ERR_NO_MESSAGE	"[\n{\"raw\":\"ERR\",\"time\":null,\"datas\":" \
	"{\"code\":\"011\",\"value\":\"ERR_NO_MESSAGE\"}}\n]\n"

#define MSG_SUC			"[\n{\"raw\":\"SUC\",\"time\":null,\"datas\":" \
	"{\"code\":\"999\",\"value\":\"OPERATION_SUCCESS\"}}\n]\n"

#define FRIEND_PIP_NAME	"*FriendPipe"

#define SET_USER_FOR_APE(ape, uin, user)								\
	do {																\
		hashtbl_append(get_property(ape->properties, "userlist")->val,	\
					   uin, user);										\
	} while (0)

#define GET_USER_FROM_APE(ape, uin)										\
	(get_property(ape->properties, "userlist") != NULL ?				\
	 hashtbl_seek(get_property(ape->properties, "userlist")->val, uin): NULL)

#define SET_UIN_FOR_USER(user, uin)					\
	do {											\
		add_property(&user->properties, "uin", uin,	\
					 EXTEND_STR, EXTEND_ISPUBLIC);	\
	} while (0)

#define GET_UIN_FROM_USER(user)							\
	(get_property(user->properties, "uin") != NULL ?	\
	 (char*)get_property(user->properties, "uin")->val: NULL)

#define GET_USER_FRIEND_TBL(user) \
	(get_property(user->properties, "friend") != NULL ? \
	 (HTBL*)get_property(user->properties, "friend")->val: NULL)

#define GET_USER_LIST													\
	(get_property(callbacki->g_ape->properties, "userlist") != NULL ?	\
	 (HTBL*)get_property(callbacki->g_ape->properties, "userlist")->val: NULL)

static ace_plugin_infos infos_module = {
	"\"Push\" system", // Module Name
	"0.01",		       // Module Version
	"hunantv",         // Module Author
	"mod_push.conf"    // config file (bin/)
};

/*
 * file range 
 */
static int isvaliduin(char *uin)
{
	if (uin == NULL)
		return 0;
        
	char *p = uin;
	while (*p != '\0') {
		if (!isdigit((int)*p))
			return 0;
		p++;
	}
	return 1;
}

static void keep_up_with_my_friend(USERS *user, acetables *g_ape)
{
	HTBL_ITEM *item;
	HTBL *ulist = GET_USER_FRIEND_TBL(user);
    char *uin = GET_UIN_FROM_USER(user);
	USERS *friend;
	CHANNEL *chan, *jchan;

    chan = getchanf(g_ape, FRIEND_PIP_NAME"%s", uin);
    if (chan == NULL) {
        wlog_err("%s baby, you didn't make a channel?", uin);
        return;
    }
    
	if (ulist != NULL)  {
		for (item = ulist->first; item != NULL; item = item->lnext) {
			/*
			 * invite my friends
			 */
			friend = GET_USER_FROM_APE(g_ape, item->key);
			if (friend != NULL) {
				wlog_dbg("%s invite %s\n", uin, item->key);
				join(friend, chan, g_ape);
			}
		
			/*
			 * join my friends
			 */
			jchan = getchanf(g_ape, FRIEND_PIP_NAME"%s", item->key);
			if (jchan != NULL) {
				join(user, jchan, g_ape);
			}
		}
	}
}

static void get_user_info(char *uin, USERS *user, acetables *g_ape)
{
	HTBL *ulist;
	extend *ext;
	
	mevent_t *evt;
	struct data_cell *pc, *cc;
	char val[64];
	
	int ret;

	if (!isvaliduin(uin) || user == NULL || g_ape == NULL) return;
	
	evt = mevent_init_plugin("uic", REQ_CMD_FRIENDLIST, FLAGS_SYNC);
	mevent_add_u32(evt, NULL, "uin", atoi(uin));
	ret = mevent_trigger(evt);
	if (!ret || ret >= REP_ERR) {
		wlog_err("get friend for user %s failure %d", uin, ret);
		goto get_msgset;
	}
	
	pc = data_cell_search(evt->rcvdata, false, DATA_TYPE_ARRAY, "friend");
	if (pc != NULL) {
		ext = add_property(&user->properties, "friend", hashtbl_init(),
						   EXTEND_HTBL, EXTEND_ISPRIVATE);
		ulist = (HTBL*)ext->val;
        iterate_data(pc) {
            cc = pc->v.aval->items[t_rsv_i];
            
			if (cc->type != DATA_TYPE_U32) continue;
			sprintf(val, "%d", cc->v.ival);
			wlog_dbg("add %s friend %s", uin, cc->key);
			hashtbl_append(ulist, (char*)cc->key, strdup(val));
		}
	}

 get_msgset:
	mevent_chose_plugin(evt, "uic", REQ_CMD_MYSETTING, FLAGS_SYNC);
	mevent_add_u32(evt, NULL, "uin", atoi(uin));
	ret = mevent_trigger(evt);
	if (!ret || ret >= REP_ERR) {
		wlog_err("get setting for user %s failure %d", uin, ret);
		goto done;
	}
	
	pc = data_cell_search(evt->rcvdata, false, DATA_TYPE_ARRAY, "incept");
	if (pc != NULL) {
		ext = add_property(&user->properties, "incept", hashtbl_init(),
						   EXTEND_HTBL, EXTEND_ISPRIVATE);
		ulist = (HTBL*)ext->val;

        iterate_data(pc) {
            cc = pc->v.aval->items[t_rsv_i];
            
			if (cc->type != DATA_TYPE_U32) continue;
			sprintf(val, "%d", cc->v.ival);
			wlog_dbg("add %s incpet %s", uin, val);
			hashtbl_append(ulist, (char*)cc->key, strdup(val));
		}
	}

	pc = data_cell_search(evt->rcvdata, false, DATA_TYPE_U32, "sitemessage");
	if (pc != NULL) {
		sprintf(val, "%d", pc->v.ival);
		wlog_dbg("add %s sitemessage %s", uin, val);
		add_property(&user->properties, "msgset", val, EXTEND_STR, EXTEND_ISPRIVATE);
	}

 done:
	mevent_free(evt);
	return;
}

/*
 * pro range
 */
static unsigned int push_connect(callbackp *callbacki)
{
	USERS *nuser, *ouser;
	RAW *newraw;
	json *jlist = NULL;
	CHANNEL *chan;

	if (isvaliduin(callbacki->param[1]) != 1) {
		SENDH(callbacki->fdclient, ERR_BAD_UIN, callbacki->g_ape);
		return (FOR_NOTHING);
	}
    ouser = GET_USER_FROM_APE(callbacki->g_ape, callbacki->param[1]);
#if 0
	if (GET_USER_FROM_APE(callbacki->g_ape, callbacki->param[1])) {
		SENDH(callbacki->fdclient, ERR_UIN_USED, callbacki->g_ape);
		return (FOR_NOTHING);
	}
#endif

	nuser = adduser(callbacki->fdclient, callbacki->host, callbacki->g_ape);
	callbacki->call_user = nuser;
	if (strcmp(callbacki->param[2], "2") == 0) {
		nuser->transport = TRANSPORT_IFRAME;
		nuser->flags |= FLG_PCONNECT;
	} else {
		nuser->transport = TRANSPORT_LONGPOLLING;
	}
	
	SET_UIN_FOR_USER(nuser, callbacki->param[1]);
	subuser_restor(getsubuser(callbacki->call_user, callbacki->host),
				   callbacki->g_ape);
    if (ouser) {
        make_link(ouser, nuser);
        nuser->flags = nuser->flags |  FLG_VUSER;

        /* post channel */
        if ((chan = getchanf(callbacki->g_ape, FRIEND_PIP_NAME"%s",
                             callbacki->param[1])) != NULL) {
            set_json("pipe", NULL, &jlist);
            json_attach(jlist, get_json_object_channel(chan), JSON_OBJECT);
            newraw = forge_raw(RAW_CHANNEL, jlist);
            post_raw(newraw, nuser, callbacki->g_ape);
        }
        goto done;
    } else {
        SET_USER_FOR_APE(callbacki->g_ape, callbacki->param[1], nuser);
        get_user_info(callbacki->param[1], nuser, callbacki->g_ape);
    }

	/*
	 * make own channel
	 */
	if ((chan = getchanf(callbacki->g_ape, FRIEND_PIP_NAME"%s",
                         callbacki->param[1])) == NULL) {
		chan = mkchanf(callbacki->g_ape, "Pipe For My Friend",
                       FRIEND_PIP_NAME"%s", callbacki->param[1]);
		if (chan == NULL) {
			smsalarm_msgf(callbacki->g_ape, "make channel %s failed",
                          callbacki->param[1]);
			SENDH(callbacki->fdclient, ERR_MAKE_CHAN, callbacki->g_ape);
			return (FOR_NOTHING);
		}
	}
	join(nuser, chan, callbacki->g_ape);
    keep_up_with_my_friend(nuser, callbacki->g_ape);

 done:
    jlist = NULL;
	set_json("sessid", nuser->sessid, &jlist);
	
	newraw = forge_raw(RAW_LOGIN, jlist);
	newraw->priority = 1;
	
	post_raw(newraw, nuser, callbacki->g_ape);

	return (FOR_LOGIN | FOR_UPDATE_IP);

	//send_raws(nuser->subuser, callbacki->g_ape);
	//do_died(nuser->subuser);
	//return (FOR_NULL);
}

static unsigned int push_send(callbackp *callbacki)
{
	json *jlist = NULL;
    USERS *user, *muser;
    CHANNEL *chan;
    char *uin;

    user = muser = callbacki->call_user;
    uin = GET_UIN_FROM_USER(user);
    
    if (user->flags & FLG_VUSER) {
        muser = GET_USER_FROM_APE(callbacki->g_ape, uin);
        if (!muser) {
            /*
             * TODO main user disconnected before visual user, how can i post them? 
             */
            wlog_warn("%s's main user may be disconnected", uin);
            goto done;
        }
    }

    /*
     * refresh main user's friends..., and join the new friend 
     */
    get_user_info(uin, muser, callbacki->g_ape);
    keep_up_with_my_friend(muser, callbacki->g_ape);

    chan = getchanf(callbacki->g_ape, FRIEND_PIP_NAME"%s", uin);
    if (chan == NULL || chan->pipe == NULL ||
        chan->pipe->type != CHANNEL_PIPE) {
        goto done;
    }
    
	set_json("msg", callbacki->param[3], &jlist);

    /*
     * 1, we can use post_to_channel here, but comply with ape, post_to_pipe
     * 2, callbacki->param[2] must be same as chan->pipe->pubid
     */
	//post_to_pipe(jlist, RAW_DATA, chan->pipe->pubid,
	//			 getsubuser(callbacki->call_user, callbacki->host),
	//			 NULL, callbacki->g_ape);
	post_to_pipe(jlist, RAW_DATA, chan->pipe->pubid,
				 muser->subuser, NULL, callbacki->g_ape);
    
 done:
	CLOSE(callbacki->fdclient, callbacki->g_ape);
	return (FOR_NULL);
}

static unsigned int push_regpageclass(callbackp *callbacki)
{
	subuser *sub = getsubuser(callbacki->call_user, callbacki->host);
	if (sub == NULL) {
		wlog_warn("get subuser failuer for %s", callbacki->host);
		SENDH(callbacki->fdclient, ERR_REGIST_APP, callbacki->g_ape);
		return (FOR_NOTHING);
	}

	char *apps = callbacki->param[2];
	if (apps == NULL) {
		wlog_warn("regpageclass NULL");
		SENDH(callbacki->fdclient, ERR_BAD_PARAM, callbacki->g_ape);
		return (FOR_NOTHING);
	}
	
	extend *ext = get_property(sub->properties, "incept");
	if (ext == NULL) {
		ext = add_property(&sub->properties, "incept", hashtbl_init(),
						   EXTEND_HTBL, EXTEND_ISPRIVATE);
	}
	
	if (ext != NULL && ext->val != NULL) {
		char *incept = strdup(apps);
		char *ids[100];
		size_t num = explode(',', incept, ids, 99);
		int loopi;
		for (loopi = 0; loopi <= num; loopi++) {
            wlog_noise("append %s for %s's %s",
                       ids[loopi], sub->user->sessid, sub->channel);
			hashtbl_append(ext->val, ids[loopi], NULL);
		}
		free(incept);

		json *jlist = NULL;
		HTBL_ITEM *item;
		HTBL *ulist = ext->val;
		set_json("pageclass", NULL, &jlist);
		for (item = ulist->first; item != NULL; item = item->lnext) {
			struct json *pageclass = NULL;
			set_json("id", item->key, &pageclass);
			json_attach(jlist, pageclass, JSON_ARRAY);
		}

		RAW *newraw = forge_raw("REGCLASS", jlist);
		post_raw_sub(newraw, sub, callbacki->g_ape);
	} else {
		wlog_err("add property error");
		SENDH(callbacki->fdclient, ERR_REGIST_APP, callbacki->g_ape);
	}
	
	return (FOR_NOTHING);
}

static unsigned int push_userlist(callbackp *callbacki)
{
	RAW *newraw;
	json *jlist = NULL;
	HTBL_ITEM *item;
	HTBL *ulist = GET_USER_LIST;
	USERS *user;
	int num;
	char tok[64];

	set_json("users", NULL, &jlist);

	num = 0;
	if (ulist != NULL) {
		for (item = ulist->first; item != NULL; item = item->lnext) {
			if (num < 20) {
				user = (USERS*)item->addrs;
				json_attach(jlist, get_json_object_user(user), JSON_ARRAY);
			}
			num++;
		}
		sprintf(tok, "%d", num);
		set_json("num", tok, &jlist);
	}

	newraw = forge_raw("USERLIST", jlist);
	newraw->priority = 1;

	post_raw(newraw, callbacki->call_user, callbacki->g_ape);
	
	return (FOR_NOTHING);
}

static unsigned int push_friendlist(callbackp *callbacki)
{
	HTBL_ITEM *item;
	HTBL *ulist = GET_USER_FRIEND_TBL(callbacki->call_user);
	
	RAW *newraw;
	json *jlist = NULL;
	USERS *friend;

	set_json("friends", NULL, &jlist);
	if (ulist != NULL) {
		for (item = ulist->first; item != NULL; item = item->lnext) {
			friend = GET_USER_FROM_APE(callbacki->g_ape, item->key);
			if (friend != NULL) {
				json_attach(jlist, get_json_object_user(friend), JSON_ARRAY);
			}
		}
	}
	newraw = forge_raw("FRIENDLIST", jlist);
	newraw->priority = 1;
		
	post_raw(newraw, callbacki->call_user, callbacki->g_ape);

	return (FOR_NOTHING);
}

/*
 * param[0]	USERONLINE
 * param[1]	14,26  OR 14
 * return ["raw":"DATA", "time":"1248446278", "datas": ["14": "1", "26": "0"]]
 */
static unsigned int push_useronline(callbackp *callbacki)
{
	RAW *newraw;
	json *jlist = NULL;
    char *users = strdup(callbacki->param[1]);
    char *uin[100];
    size_t num = explode(',', users, uin, 99);
    
    int loopi;
    for (loopi = 0; loopi <= num; loopi++) {
        if (isvaliduin(uin[loopi])) {
            if (GET_USER_FROM_APE(callbacki->g_ape, uin[loopi])) {
                set_json(uin[loopi], "1", &jlist);
            } else {
                set_json(uin[loopi], "0", &jlist);
            }
        }
    }

    free(users);

    newraw = forge_raw(RAW_DATA, jlist);
    newraw->priority = 1;

    sendbin(callbacki->fdclient, HEADER, HEADER_LEN, callbacki->g_ape);
    sendbin(callbacki->fdclient, "[\n", 2, callbacki->g_ape);
    sendbin(callbacki->fdclient, newraw->data, newraw->len, callbacki->g_ape);
    sendbin(callbacki->fdclient, "\n]\n", 3, callbacki->g_ape);
    free(newraw->data);
    free(newraw);
    
	return (FOR_NULL);
}

/*
 * param[0]	SENDUNIQ
 * param[1]	SESSIONID
 * param[2]	TARGETUSER
 * param[3]	MESSAGE
 */
static unsigned int push_senduniq(callbackp *callbacki)
{
	USERS *user = GET_USER_FROM_APE(callbacki->g_ape, callbacki->param[2]);

	if (user == NULL) {
		wlog_info("get user failuer %s", callbacki->param[2]);
		SENDH(callbacki->fdclient, ERR_UIN_NEXIST, callbacki->g_ape);
		return (FOR_NULL);
	}

	/*
	 * target user may set 1: accept 2: reject 3: accept friend
	 */
#if 0
	json_item *oitem, *pitem;
	oitem = init_json_parser(callbacki->param[3]);
	if (oitem == NULL) {
		SENDH(callbacki->fdclient, ERR_BAD_PARAM, callbacki->g_ape);
		return (FOR_NULL);
	}
	while (oitem != NULL) {
		if (strcmp(oitem->key, "pageclass")) {
			if (oitem->jval.vu.interger_value == 1) {
				extend *ext = get_property(user->properties, "msgset");
				if (ext != NULL && ext->val != NULL) {
					if (!strcmp(ext->val, "2")) {
						wlog_info("%s reject message", callbacki->param[2]);
						SENDH(callbacki->fdclient, ERR_USER_REFUSE, callbacki->g_ape);
						return (FOR_NULL);
					} else if (!strcmp(ext->val, "3")) {
						wlog_info("user %s just rcv friend's message",
								  callbacki->param[2]);
						char *sender = GET_UIN_FROM_USER(callbacki->call_user);
						if (sender != NULL) {
							HTBL *list = GET_USER_FRIEND_TBL(user);
							if (list != NULL) {
								if (hashtbl_seek(list, sender) == NULL) {
									SENDH(callbacki->fdclient, ERR_USER_REFUSE,
										  callbacki->g_ape);
									return (FOR_NULL);
								}
							} else {
								SENDH(callbacki->fdclient, ERR_USER_REFUSE,
									  callbacki->g_ape);
								return (FOR_NULL);
							}
						} else {
							SENDH(callbacki->fdclient, ERR_USER_REFUSE,
								  callbacki->g_ape);
							return (FOR_NULL);
						}
						wlog_info("user %s is %s's friend", sender,
								  callbacki->param[2]);
					}
				}
			}
			break;				/* pageclass supplied and accpeted! */
		}
		oitem = pitem->next;
	}
	if (oitem == NULL) {
		SENDH(callbacki->fdclient, ERR_BAD_PARAM, callbacki->g_ape);
		return (FOR_NULL);
	}
#endif
	
	if (strstr(callbacki->param[3], "pageclass%3A1") != NULL) {
		extend *ext = get_property(user->properties, "msgset");
		if (ext != NULL && ext->val != NULL) {
			if (!strcmp(ext->val, "2")) {
				wlog_info("%s reject message", callbacki->param[2]);
				SENDH(callbacki->fdclient, ERR_USER_REFUSE, callbacki->g_ape);
				return (FOR_NULL);
			} else if (!strcmp(ext->val, "3")) {
				wlog_info("user %s just rcv friend's message", callbacki->param[2]);
				char *sender = GET_UIN_FROM_USER(callbacki->call_user);
				if (sender != NULL) {
					HTBL *list = GET_USER_FRIEND_TBL(user);
					if (list != NULL) {
						if (hashtbl_seek(list, sender) == NULL) {
							SENDH(callbacki->fdclient, ERR_USER_REFUSE,
								  callbacki->g_ape);
							return (FOR_NULL);
						}
					} else {
						SENDH(callbacki->fdclient, ERR_USER_REFUSE, callbacki->g_ape);
						return (FOR_NULL);
					}
				} else {
					SENDH(callbacki->fdclient, ERR_USER_REFUSE, callbacki->g_ape);
					return (FOR_NULL);
				}
				wlog_info("user %s is %s's friend", sender, callbacki->param[2]);
			}
		}
	}

	json *jlist = NULL;
	RAW *newraw;

	set_json("msg", callbacki->param[3], &jlist);
	newraw = forge_raw(RAW_DATA, jlist);
	newraw->priority = 1;

	post_raw(newraw, user, callbacki->g_ape);

	SENDH(callbacki->fdclient, MSG_SUC, callbacki->g_ape);
	return (FOR_NULL);
}

static void push_deluser(USERS *user, acetables *g_ape)
{
	CHANNEL *chan;
	char *uin = GET_UIN_FROM_USER(user);

    if (user->flags & FLG_VUSER) {
        goto normal_del;
    } else if (user->links.nlink != 0) {
        /*
         * don't del main user while visual user exist 
         */
        return;
    }

	chan = getchanf(g_ape, FRIEND_PIP_NAME"%s", uin);
	if (chan != NULL) {
		HTBL *ulist = GET_USER_FRIEND_TBL(user);
		HTBL_ITEM *item;
		USERS *friend;
		RAW *newraw;
		json *jlist = NULL;
		if (ulist != NULL) {
			for (item = ulist->first; item != NULL; item = item->lnext) {
				friend = GET_USER_FROM_APE(g_ape, item->key);
				if (friend != NULL) {
					jlist = NULL;
					set_json("user", NULL, &jlist);
					json_attach(jlist, get_json_object_user(user), JSON_OBJECT);
					newraw = forge_raw(RAW_LEFT, jlist);
					newraw->priority = 1;
					post_raw(newraw, friend, g_ape);
					
					left(friend, chan, g_ape);
				}
			}
		}
	}

	if (uin != NULL && get_property(g_ape->properties, "userlist") != NULL) {
		hashtbl_erase(get_property(g_ape->properties, "userlist")->val, uin);
	}

 normal_del:
	left_all(user, g_ape);
	
	/* kill all users connections */
	
	clear_subusers(user);

    if (user->links.nlink != 0 && user->links.ulink) {
        struct _link_list *cur, *next;
        cur = user->links.ulink;
        next = user->links.ulink->next;
        while (cur) {
            destroy_link(cur->link->a, cur->link->b);
            cur = next;
            if (next) next = next->next;
        }
    }
    
	hashtbl_erase(g_ape->hSessid, user->sessid);
	
	g_ape->nConnected--;

	
	if (user->prev == NULL) {
		g_ape->uHead = user->next;
	} else {
		user->prev->next = user->next;
	}
	if (user->next != NULL) {
		user->next->prev = user->prev;
	}

	clear_sessions(user);
	clear_properties(&user->properties);
	destroy_pipe(user->pipe, g_ape);

	free(user);

	user = NULL;
}

static void push_post_raw_sub(RAW *raw, subuser *sub, acetables *g_ape)
{
	extend *usrp = get_property(sub->user->properties, "incept");
	extend *subp = get_property(sub->properties, "incept");
	extend *msgp = get_property(sub->user->properties, "msgset");
	
	char tok[64];
	int post;

	HTBL *list;
	HTBL_ITEM *item;

	wlog_noise("prepare post %s", raw->data);
	
	/*
	 * if not a DATA message, post anyway
	 */
	if (strstr(raw->data, "\"raw\":\"DATA\"") == NULL)
		goto do_post;
	
	/*
	 * if not a feed message, just check pageclass 
	 */
	if (strstr(raw->data, "pageclass%3A3") == NULL)
		goto judge_class;
	

	/*
	 * check type
	 */
	if (usrp != NULL && usrp->val != NULL) {
		post = 0;
		list = usrp->val;
		for (item = list->first; item != NULL; item = item->lnext) {
			snprintf(tok, sizeof(tok), "type%%3A%s", item->key);
			if (strstr(raw->data, tok) != NULL) {
				post = 1;
				break;
			}
		}
		if (post == 0) return;
	}

 judge_class:
	/*
	 * check msgset
	 */
	if (msgp != NULL && msgp->val != NULL) {
		if (strstr(raw->data, "pageclass%3A1") != NULL) {
			if (!strcmp(msgp->val, "2")) {
				return;
			}
		}
	}
	
	/*
	 * check pageclass
	 */
	if (subp != NULL && subp->val != NULL) {
		post = 0;
		list = subp->val;
		for (item = list->first; item != NULL; item = item->lnext) {
			snprintf(tok, sizeof(tok), "pageclass%%3A%s", item->key);
			if (strstr(raw->data, tok) != NULL) {
				post = 1;
				break;
			}
		}
		if (post == 0) return;
	}

 do_post:
	if (raw->priority == 0) {
		if (sub->rawhead == NULL) {
			sub->rawhead = raw;
		}
		if (sub->rawfoot != NULL) {
			sub->rawfoot->next = raw;
		}
		sub->rawfoot = raw;
	} else {
		
		if (sub->rawfoot == NULL) {
			sub->rawfoot = raw;
		}		
		raw->next = sub->rawhead;
		sub->rawhead = raw;
	}
	(sub->nraw)++;
	wlog_noise("%s done", raw->data);
}

static unsigned int push_trustcheck(callbackp *callbacki)
{
	USERS *user;

	user = GET_USER_FROM_APE(callbacki->g_ape, callbacki->param[1]);

	if (user == NULL) {
		SENDH(callbacki->fdclient, ERR_UIN_NEXIST, callbacki->g_ape);
		return (FOR_NULL);
	}

	subuser *sub;
	RAW *raw, *older;
	int finish = 1;
	
	sub = user->subuser;
	raw = sub->rawhead;
	
	if (raw != NULL) {
		sendbin(callbacki->fdclient, HEADER, HEADER_LEN, callbacki->g_ape);
		finish &= sendbin(callbacki->fdclient, "[\n", 2, callbacki->g_ape);
	} else {
		SENDH(callbacki->fdclient, ERR_NO_MESSAGE, callbacki->g_ape);
		return (FOR_NULL);
	}
	while(raw != NULL) {

		finish &= sendbin(callbacki->fdclient, raw->data, raw->len, callbacki->g_ape);
		
		if (raw->next != NULL) {
			finish &= sendbin(callbacki->fdclient, ",\n", 2, callbacki->g_ape);
		} else {
			finish &= sendbin(callbacki->fdclient, "\n]\n", 3, callbacki->g_ape);	
		}
		older = raw;
		raw = raw->next;
		
		free(older->data);
		free(older);
	}
	
	sub->rawhead = NULL;
	sub->rawfoot = NULL;
	sub->nraw = 0;

	return (FOR_NULL);
	
#if 0
	addsubuser(callbacki->fdclient, callbacki->host, user, callbacki->g_ape);
	callbacki->call_user = user;
	return (FOR_LOGIN);
#endif
}

static unsigned int push_trustsend(callbackp *callbacki)
{
	USERS *user = GET_USER_FROM_APE(callbacki->g_ape, callbacki->param[1]);

	if (user == NULL) {
		SENDH(callbacki->fdclient, ERR_UIN_NEXIST, callbacki->g_ape);
		return (FOR_NULL);
	}

	RAW *newraw;
	json *jlist = NULL;

	set_json("msg", callbacki->param[2], &jlist);
	newraw = forge_raw(RAW_DATA, jlist);
	newraw->priority = 1;
	
	post_raw(newraw, user, callbacki->g_ape);

	SENDH(callbacki->fdclient, MSG_SUC, callbacki->g_ape);
	return (FOR_NULL);
}

static void init_module(acetables *g_ape)
{
	add_property(&g_ape->properties, "userlist", hashtbl_init(),
				 EXTEND_POINTER, EXTEND_ISPRIVATE);
	register_cmd("CONNECT",	2, push_connect, NEED_NOTHING, g_ape);
	register_cmd("SEND",	3, push_send, NEED_SESSID, g_ape);
	register_cmd("REGCLASS", 2, push_regpageclass, NEED_SESSID, g_ape);
	register_cmd("USERLIST", 1, push_userlist, NEED_SESSID, g_ape);
	register_cmd("FRIENDLIST", 1, push_friendlist, NEED_SESSID, g_ape);
	register_cmd("USERONLINE", 1, push_useronline, NEED_NOTHING, g_ape);
	register_cmd("SENDUNIQ", 3, push_senduniq, NEED_SESSID, g_ape);
	register_cmd("TRUSTCHECK", 1, push_trustcheck, NEED_NOTHING, g_ape);
	register_cmd("TRUSTSEND", 2, push_trustsend, NEED_NOTHING, g_ape);
}

static ace_callbacks callbacks = {
	NULL,				/* Called when new user is added */
	push_deluser,		/* Called when a user is disconnected */
	NULL,				/* Called when new chan is created */
	NULL,				/* Called when a user join a channel */
	NULL,				/* Called when a user leave a channel */
	NULL,				/* Called at each tick, passing a subuser */
	push_post_raw_sub	/* Called when a subuser receiv a message */
};

APE_INIT_PLUGIN(MODULE_NAME, init_module, callbacks)
