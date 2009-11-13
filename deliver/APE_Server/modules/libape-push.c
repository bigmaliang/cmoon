#include "plugins.h"
#include "config.h"
#include <mysql/mysql.h>

#include "mevent.h"
#include "data.h"
#include "mevent_uic.h"

#define MODULE_NAME "push"

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

#define GET_UIN_FROM_USER(user)                                 \
	(get_property(user->properties, "uin") != NULL ?            \
	 (char*)get_property(user->properties, "uin")->val: NULL)

#define GET_USER_FRIEND_TBL(user)                                   \
	(get_property(user->properties, "friend") != NULL ?             \
	 (HTBL*)get_property(user->properties, "friend")->val: NULL)

#define GET_USER_LIST													\
	(get_property(callbacki->g_ape->properties, "userlist") != NULL ?	\
	 (HTBL*)get_property(callbacki->g_ape->properties, "userlist")->val: NULL)

static ace_plugin_infos infos_module = {
	"\"Push\" system", // Module Name
	"1.0",		       // Module Version
	"hunantv",         // Module Author
	"mod_push.conf"    // config file (bin/)
};

/*
 * file range 
 */
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
				wlog_noise("%s invite %s\n", uin, item->key);
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

	if (!hn_isvaliduin(uin) || user == NULL || g_ape == NULL) return;
	
	evt = mevent_init_plugin("uic", REQ_CMD_FRIENDLIST, FLAGS_SYNC);
	mevent_add_u32(evt, NULL, "uin", atoi(uin));
	ret = mevent_trigger(evt);
	if (PROCESS_NOK(ret)) {
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
			wlog_noise("add %s friend %s", uin, cc->key);
			hashtbl_append(ulist, (char*)cc->key, strdup(val));
		}
	}

 get_msgset:
	mevent_chose_plugin(evt, "uic", REQ_CMD_MYSETTING, FLAGS_SYNC);
	mevent_add_u32(evt, NULL, "uin", atoi(uin));
	ret = mevent_trigger(evt);
	if (PROCESS_NOK(ret)) {
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
			wlog_noise("add %s incpet %s", uin, val);
			hashtbl_append(ulist, (char*)cc->key, strdup(val));
		}
	}

	pc = data_cell_search(evt->rcvdata, false, DATA_TYPE_U32, "sitemessage");
	if (pc != NULL) {
		sprintf(val, "%d", pc->v.ival);
		wlog_noise("add %s sitemessage %s", uin, val);
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
	json_item *jlist = NULL;
	CHANNEL *chan;
    char *uin;

    JNEED_STR(callbacki->param, "uin", uin);
    
	if (!hn_isvaliduin(uin)) {
		return (RETURN_BAD_PARAMS);
	}
    ouser = GET_USER_FROM_APE(callbacki->g_ape, uin);

	nuser = adduser(callbacki->client, callbacki->host,
                    callbacki->properties, callbacki->ip, callbacki->g_ape);
	if (nuser == NULL) {
		jlist = json_new_object();
		json_set_property_strZ(jlist, "code", "200");
		json_set_property_strZ(jlist, "value", "UNKNOWN_CONNECTION_ERROR");
		newraw = forge_raw(RAW_ERR, jlist);
		send_raw_inline(callbacki->client, callbacki->transport, newraw, callbacki->g_ape);
		
		clear_properties(&callbacki->properties);
		
		return (RETURN_NOTHING);
	}
    
	callbacki->call_user = nuser;
	switch(callbacki->transport) {
    case 1:
        nuser->transport = TRANSPORT_XHRSTREAMING;
        break;
    case 2:
        nuser->transport = TRANSPORT_JSONP;
        break;
    case 3:
        nuser->transport = TRANSPORT_PERSISTANT;
        break;
    case 4:
        nuser->transport = TRANSPORT_SSE_LONGPOLLING;
        break;
    case 5:
        nuser->transport = TRANSPORT_SSE_JSONP;
        break;
    default:
        nuser->transport = TRANSPORT_LONGPOLLING;
        break;
		
	}
	
	SET_UIN_FOR_USER(nuser, uin);
	subuser_restor(getsubuser(callbacki->call_user, callbacki->host),
				   callbacki->g_ape);
    if (ouser) {
        make_link(ouser, nuser);
        nuser->flags = nuser->flags | FLG_VUSER;

        /* post channel */
        if ((chan = getchanf(callbacki->g_ape, FRIEND_PIP_NAME"%s", uin)) != NULL) {
            jlist = json_new_object();
            json_set_property_objZ(jlist, "pipe", get_json_object_channel(chan));
            newraw = forge_raw(RAW_CHANNEL, jlist);
            post_raw(newraw, nuser, callbacki->g_ape);
        }
        goto done;
    } else {
        SET_USER_FOR_APE(callbacki->g_ape, uin, nuser);
        get_user_info(uin, nuser, callbacki->g_ape);
    }

	/*
	 * make own channel
	 */
	if ((chan = getchanf(callbacki->g_ape, FRIEND_PIP_NAME"%s", uin)) == NULL) {
		chan = mkchanf(callbacki->g_ape, FRIEND_PIP_NAME"%s", uin);
		if (chan == NULL) {
            wlog_err("make channel %s failure", uin);
			//smsalarm_msgf(callbacki->g_ape, "make channel %s failed", uin);
            hn_senderr(callbacki, "007", "ERR_MAKE_CHANNEL");
			return (RETURN_NOTHING);
		}
	}
	join(nuser, chan, callbacki->g_ape);
    keep_up_with_my_friend(nuser, callbacki->g_ape);

 done:
    jlist = json_new_object();
    json_set_property_strZ(jlist, "sessid", nuser->sessid);
	newraw = forge_raw(RAW_LOGIN, jlist);
	newraw->priority = RAW_PRI_HI;
	post_raw(newraw, nuser, callbacki->g_ape);

	return (RETURN_LOGIN | RETURN_UPDATE_IP);

	//send_raws(nuser->subuser, callbacki->g_ape);
	//do_died(nuser->subuser);
	//return (FOR_NULL);
}

static unsigned int push_send(callbackp *callbacki)
{
	json_item *jlist = NULL;
    RAW *newraw;
    USERS *user, *muser;
    CHANNEL *chan;
    char *uin;
    json_item *msg;

    user = muser = callbacki->call_user;
    uin = GET_UIN_FROM_USER(user);
    JNEED_OBJ(callbacki->param, "msg", msg);
    
    if (user->flags & FLG_VUSER) {
        muser = GET_USER_FROM_APE(callbacki->g_ape, uin);
        if (!muser) {
            /*
             * TODO main user disconnected before visual user, how can i post them?
             * FIXED: main user won't delete except all its vuser deleted
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
        wlog_warn("channel %s %s not exist", FRIEND_PIP_NAME, uin);
        goto done;
    }

    jlist = json_new_object();
    json_item *jcopy = json_item_copy(msg->father, NULL);
    //json_set_property_strZ(jlist, "msg", msg);
    json_set_property_objZ(jlist, "msg", jcopy);

    json_set_property_objZ(jlist, "pipe", get_json_object_channel(chan));
    newraw = forge_raw(RAW_DATA, jlist);
    post_raw_channel_restricted(newraw, chan, muser, callbacki->g_ape);

    st_push *st = (st_push*)get_property(callbacki->g_ape->properties,
                                         "msgstatic")->val;
    st->msg_feed++;

#if 0
    /*
     * 1, we can use post_to_channel here, but comply with ape, post_to_pipe
     * 2, callbacki->param[2] must be same as chan->pipe->pubid
     */
	//post_to_pipe(jlist, RAW_DATA, chan->pipe->pubid,
	//			 getsubuser(callbacki->call_user, callbacki->host),
	//			 NULL, callbacki->g_ape);
	post_to_pipe(jlist, RAW_DATA, chan->pipe->pubid,
				 muser->subuser, callbacki->g_ape);
#endif
    
 done:
	//CLOSE(callbacki->fdclient, callbacki->g_ape);
    jlist = json_new_object();
    json_set_property_strZ(jlist, "value", "null");
    RAW *raw = forge_raw("CLOSE", jlist);
    send_raw_inline(callbacki->client, callbacki->transport, raw, callbacki->g_ape);
    
	return (RETURN_NULL);
}

static unsigned int push_regpageclass(callbackp *callbacki)
{
	subuser *sub = getsubuser(callbacki->call_user, callbacki->host);
	if (sub == NULL) {
		wlog_warn("get subuser failuer for %s", callbacki->host);
        hn_senderr(callbacki, "008", "ERR_REG_APP");
		return (RETURN_NOTHING);
	}

	char *apps;
    JNEED_STR(callbacki->param, "apps", apps);
	
	extend *ext = get_property(sub->properties, "incept");
	if (ext == NULL) {
		ext = add_property(&sub->properties, "incept", hashtbl_init(),
						   EXTEND_HTBL, EXTEND_ISPRIVATE);
	}
	
	if (ext != NULL && ext->val != NULL) {
		char *incept = strdup(apps);
		char *ids[100];
		size_t num = explode('x', incept, ids, 99);
		int loopi;
		for (loopi = 0; loopi <= num; loopi++) {
            wlog_noise("append %s for %s's %s",
                       ids[loopi], sub->user->sessid, sub->channel);
			hashtbl_append(ext->val, ids[loopi], NULL);
		}
		free(incept);

		json_item *jlist = json_new_object();
        json_item *class_list = json_new_array();
		HTBL_ITEM *item;
		HTBL *ulist = ext->val;
		for (item = ulist->first; item != NULL; item = item->lnext) {
			json_item *class = json_new_object();
            json_set_property_strZ(class, "id", item->key);
			json_set_element_obj(class_list, class);
		}
        json_set_property_objZ(jlist, "pageclass", class_list);

		RAW *newraw = forge_raw("REGCLASS", jlist);
		post_raw_sub(newraw, sub, callbacki->g_ape);
	} else {
		wlog_err("add property error");
        hn_senderr(callbacki, "008", "ERR_REG_APP");
	}
	
	return (RETURN_NOTHING);
}

static unsigned int push_userlist(callbackp *callbacki)
{
	RAW *newraw;
	json_item *jlist = json_new_object();
	HTBL_ITEM *item;
	HTBL *ulist = GET_USER_LIST;
	USERS *user;
	int num;

    json_item *user_list = json_new_array();
    json_set_property_objZ(jlist, "users", user_list);

	num = 0;
	if (ulist != NULL) {
		for (item = ulist->first; item != NULL; item = item->lnext) {
			if (num < 20) {
				user = (USERS*)item->addrs;
                json_set_element_obj(user_list, get_json_object_user(user));
			}
			num++;
		}
        json_set_property_intZ(jlist, "num", num);
	}

	newraw = forge_raw("USERLIST", jlist);
	post_raw(newraw, callbacki->call_user, callbacki->g_ape);
	
	return (RETURN_NOTHING);
}

static unsigned int push_friendlist(callbackp *callbacki)
{
	HTBL_ITEM *item;
	HTBL *ulist = GET_USER_FRIEND_TBL(callbacki->call_user);
	
	RAW *newraw;
	json_item *jlist = json_new_object();
	USERS *friend;

    json_item *frd_list = json_new_array();
    json_set_property_objZ(jlist, "friends", frd_list);
	if (ulist != NULL) {
		for (item = ulist->first; item != NULL; item = item->lnext) {
			friend = GET_USER_FROM_APE(callbacki->g_ape, item->key);
			if (friend != NULL) {
                json_set_element_obj(frd_list, get_json_object_user(friend));
			}
		}
	}
	newraw = forge_raw("FRIENDLIST", jlist);
	post_raw(newraw, callbacki->call_user, callbacki->g_ape);

	return (RETURN_NOTHING);
}

static unsigned int push_useronline(callbackp *callbacki)
{
	RAW *newraw;
	json_item *jlist = json_new_object();
    char *users;
    char *uin[100];
    size_t num = explode(',', users, uin, 99);

    JNEED_STR_CPY(callbacki->param, "users", users);
    
    int loopi;
    for (loopi = 0; loopi <= num; loopi++) {
        if (hn_isvaliduin(uin[loopi])) {
            if (GET_USER_FROM_APE(callbacki->g_ape, uin[loopi])) {
                json_set_property_strZ(jlist, uin[loopi], "1");
            } else {
                json_set_property_strZ(jlist, uin[loopi], "0");
            }
        }
    }

    free(users);

    newraw = forge_raw(RAW_DATA, jlist);
	post_raw(newraw, callbacki->call_user, callbacki->g_ape);
    
	return (RETURN_NULL);
}

static unsigned int push_senduniq(callbackp *callbacki)
{
    char *dstuin;
    json_item *msg;

    JNEED_STR(callbacki->param, "dstuin", dstuin);
    JNEED_OBJ(callbacki->param, "msg", msg);
    
	USERS *user = GET_USER_FROM_APE(callbacki->g_ape, dstuin);

	if (user == NULL) {
		wlog_info("get user failuer %s", dstuin);
        hn_senderr(callbacki, "009", "ERR_UIN_NEXIST");
		return (RETURN_NULL);
	}

    st_push *st = (st_push*)get_property(callbacki->g_ape->properties,
                                         "msgstatic")->val;
    st->msg_notice++;
	/*
	 * target user may set 1: accept 2: reject 3: accept friend
     * senduniq must contain pageclass: 1
	 */
    json_item *it = json_lookup(msg, "pageclass");
    extend *ext = get_property(user->properties, "msgset");
    if (it != NULL && it->jval.vu.integer_value == 1 &&
        ext != NULL && ext->val != NULL) {
        if (!strcmp(ext->val, "2")) {
            wlog_info("%s reject message", dstuin);
            hn_senderr(callbacki, "010", "ERR_USER_REFUSE");
            return (RETURN_NULL);
        } else if (!strcmp(ext->val, "3")) {
            wlog_info("user %s just rcv friend's message", dstuin);
            char *sender = GET_UIN_FROM_USER(callbacki->call_user);
            if (sender != NULL) {
                HTBL *list = GET_USER_FRIEND_TBL(user);
                if (list != NULL) {
                    if (hashtbl_seek(list, sender) == NULL) {
                        hn_senderr(callbacki, "012", "ERR_NOT_FRIEND");
                        return (RETURN_NULL);
                    }
                } else {
                    hn_senderr(callbacki, "012", "ERR_NOT_FRIEND");
                    return (RETURN_NULL);
                }
            } else {
                hn_senderr(callbacki, "009", "ERR_UIN_NEXIST");
                return (RETURN_NULL);
            }
            wlog_warn("user %s not exist", dstuin);
        }
    }

    json_item *jlist = json_new_object();
    /* TODO fuck, why should i copy msg->father rather than msg????? */
    json_item *jcopy = json_item_copy(msg->father, NULL);
	RAW *newraw;

    //wlog_dbg("jcopy is %s", json_to_string(jcopy, NULL, 0)->jstring);
    json_set_property_objZ(jlist, "msg", jcopy);
    //wlog_dbg("jlist is %s", json_to_string(jlist, NULL, 0)->jstring);
    
	newraw = forge_raw(RAW_DATA, jlist);
	post_raw(newraw, user, callbacki->g_ape);

    json_item *ej = json_new_object();
    json_set_property_strZ(ej, "code", "999");
    json_set_property_strZ(ej, "value", "OPERATION_SUCESS");
    newraw = forge_raw(RAW_DATA, ej);
    send_raw_inline(callbacki->client, callbacki->transport,
                    newraw, callbacki->g_ape);
    
	return (RETURN_NULL);
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
		json_item *jlist;
		if (ulist != NULL) {
			for (item = ulist->first; item != NULL; item = item->lnext) {
				friend = GET_USER_FROM_APE(g_ape, item->key);
				if (friend != NULL) {
                    jlist = json_new_object();
                    json_set_property_objZ(jlist, "user", get_json_object_user(user));
					newraw = forge_raw(RAW_LEFT, jlist);
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
	
	clear_subusers(user, g_ape);

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
	
	int post;

	HTBL *list;
	HTBL_ITEM *item;
	int add_size = 16;
	struct _raw_pool_user *pool;
    
	wlog_noise("prepare post %s", raw->data);

	/*
	 * if not a DATA message, post anyway
	 */
    json_item *oit, *it;
    char *rawtype;

    oit = it = NULL;
    oit = init_json_parser(raw->data);
    if (oit != NULL) it = oit->jchild.child;
    if (it == NULL) {
        wlog_err("%s not json format", raw->data);
        return;
    }
    JNEED_STR_VOID(it, "raw", rawtype);
    
    if (strcmp(rawtype, "DATA"))
        goto do_post;

	/*
	 * if not a feed message, just check pageclass 
	 */
    int pageclass;
    JNEED_INT_VOID(it, "data.msg.pageclass", pageclass);
    if (pageclass != 3)
		goto judge_class;
	

	/*
	 * check type
	 */
    int type;
    JNEED_INT_VOID(it, "data.msg.type", type);
	if (usrp != NULL && usrp->val != NULL) {
		post = 0;
		list = usrp->val;
		for (item = list->first; item != NULL; item = item->lnext) {
			if (type == atoi(item->key)) {
				post = 1;
				break;
			}
		}
		if (post == 0) {
            wlog_dbg("data's type %d not in my setting", type);
            return;
        }
	}

 judge_class:
	/*
	 * check msgset
	 */
	if (msgp != NULL && msgp->val != NULL) {
		if (pageclass == 1) {
			if (!strcmp(msgp->val, "2")) {
                wlog_dbg("user set don't accept pageclass=1");
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
			if (pageclass == atoi(item->key)) {
				post = 1;
				break;
			}
		}
		if (post == 0) {
            wlog_dbg("pageclass %d not in my properties(sub)", pageclass);
            return;
        }
	}

 do_post:
    //free_json_item(oit);
    pool = (raw->priority == RAW_PRI_LO ?
            &sub->raw_pools.low : &sub->raw_pools.high);
    
	if (++pool->nraw == pool->size) {
		pool->size += add_size;
		expend_raw_pool(pool->rawfoot, add_size);
	}
	
	pool->rawfoot->raw = raw;
	pool->rawfoot = pool->rawfoot->next;
	
	(sub->raw_pools.nraw)++;
	wlog_noise("%s done", raw->data);
}

static unsigned int push_trustsend(callbackp *callbacki)
{
    char *dstuin, *msg;

    JNEED_STR(callbacki->param, "dstuin", dstuin);
    JNEED_STR(callbacki->param, "msg", msg);
    
	USERS *user = GET_USER_FROM_APE(callbacki->g_ape, dstuin);

	if (user == NULL) {
        hn_senderr(callbacki, "009", "ERR_UIN_NEXIST");
		return (RETURN_NULL);
	}

	RAW *newraw;
	json_item *jlist = json_new_object();

    json_set_property_strZ(jlist, "msg", msg);
	newraw = forge_raw(RAW_DATA, jlist);
	post_raw(newraw, user, callbacki->g_ape);

    json_item *ej = json_new_object();
    json_set_property_strZ(ej, "code", "999");
    json_set_property_strZ(ej, "value", "OPERATION_SUCESS");
    newraw = forge_raw(RAW_DATA, ej);
    send_raw_inline(callbacki->client, callbacki->transport,
                    newraw, callbacki->g_ape);
    
	return (RETURN_NULL);
}

static void init_module(acetables *g_ape)
{
    st_push *stdata = xmalloc(sizeof(st_push));
	add_property(&g_ape->properties, "userlist", hashtbl_init(),
				 EXTEND_POINTER, EXTEND_ISPRIVATE);
    add_property(&g_ape->properties, "msgstatic", stdata,
                 EXTEND_POINTER, EXTEND_ISPRIVATE);
	register_cmd("CONNECT",	push_connect, NEED_NOTHING, g_ape);
	register_cmd("SEND", push_send, NEED_SESSID, g_ape);
	register_cmd("REGCLASS", push_regpageclass, NEED_SESSID, g_ape);
	register_cmd("USERLIST", push_userlist, NEED_SESSID, g_ape);
	register_cmd("FRIENDLIST", push_friendlist, NEED_SESSID, g_ape);
	//register_cmd("USERONLINE", push_useronline, NEED_NOTHING, g_ape);
	register_cmd("USERONLINE", push_useronline, NEED_SESSID, g_ape);
	register_cmd("SENDUNIQ", push_senduniq, NEED_SESSID, g_ape);
	//register_cmd("TRUSTCHECK", push_trustcheck, NEED_NOTHING, g_ape);
	register_cmd("TRUSTSEND", push_trustsend, NEED_NOTHING, g_ape);
}

static ace_callbacks callbacks = {
	NULL,				/* Called when new user is added */
	push_deluser,		/* Called when a user is disconnected */
    NULL,               /* Called when a subuser is disconnected */
	NULL,				/* Called when new chan is created */
    NULL,               /* Called when a chan removed */
	NULL,				/* Called when a user join a channel */
	NULL,				/* Called when a user leave a channel */
	NULL,				/* Called at each tick, passing a subuser */
	push_post_raw_sub	/* Called when a subuser receiv a message */
};

APE_INIT_PLUGIN(MODULE_NAME, init_module, callbacks)
