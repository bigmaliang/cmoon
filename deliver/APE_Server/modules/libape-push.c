#include "plugins.h"
#include "config.h"
#include <mysql/mysql.h>
#include "mevent.h"
#include "data.h"

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

static void init_user_info(char *uin, USERS *user, acetables *g_ape)
{
	if (!isvaliduin(uin) || user == NULL || g_ape == NULL) return;

	MYSQL_RES *res;
	MYSQL_ROW row;
	HTBL *ulist;
	extend *ext;
	
	res = ape_mysql_selectf(g_ape, "userinfo", "SELECT incept_dynamic, receive_msg "
							" FROM relation.user_info WHERE userid=%s;", uin);
	if (res == NULL) {
		goto init_friend;
	}
	if ((row = mysql_fetch_row(res)) == NULL) {
		mysql_free_result(res);
		goto init_friend;
	}
	if (!strcmp(row[0], "")) {
		goto init_msgset;
	}

	/*
	 * replace incpet pro if exist 
	 */
	ext = add_property(&user->properties, "incept", hashtbl_init(),
					   EXTEND_HTBL, EXTEND_ISPRIVATE);
	ulist = (HTBL*)ext->val;

	char *incept = strdup(row[0]);
	char *ids[100];
	size_t num = explode(',', incept, ids, 99);
	int loopi;
	for (loopi = 0; loopi <= num; loopi++) {
		hashtbl_append(ulist, ids[loopi], strdup(ids[loopi]));
		//hashtbl_append(ulist, ids[loopi], NULL);
	}
	free(incept);

 init_msgset:
	add_property(&user->properties, "msgset", row[1], EXTEND_STR, EXTEND_ISPRIVATE);
	mysql_free_result(res);

 init_friend:
	res = ape_mysql_selectf(g_ape, "userinfo", "SELECT friend_userid FROM "
							" relation.user_friends WHERE userid=%s;", uin);
	if (res == NULL)
		return;
	
	/*
	 * replace friend pro if exist 
	 */
	ext = add_property(&user->properties, "friend", hashtbl_init(),
					   EXTEND_HTBL, EXTEND_ISPRIVATE);
	ulist = (HTBL*)ext->val;
	while ((row = mysql_fetch_row(res)) != NULL) {
		hashtbl_append(ulist, row[0], strdup(row[0]));
		//hashtbl_append(ulist, row[0], NULL);
	}
	mysql_free_result(res);
}

/*
 * pro range
 */
static unsigned int push_connect(callbackp *callbacki)
{
	USERS *nuser;
	RAW *newraw;
	json *jlist = NULL;
	
	if (isvaliduin(callbacki->param[1]) != 1) {
		SENDH(callbacki->fdclient, ERR_BAD_UIN, callbacki->g_ape);
		return (FOR_NOTHING);
	}
	if (GET_USER_FROM_APE(callbacki->g_ape, callbacki->param[1])) {
		SENDH(callbacki->fdclient, ERR_UIN_USED, callbacki->g_ape);
		return (FOR_NOTHING);
	}

	nuser = adduser(callbacki->fdclient, callbacki->host, callbacki->g_ape);
	callbacki->call_user = nuser;
	if (strcmp(callbacki->param[2], "2") == 0) {
		nuser->transport = TRANSPORT_IFRAME;
		nuser->flags |= FLG_PCONNECT;
	} else {
		nuser->transport = TRANSPORT_LONGPOLLING;
	}
	
	SET_UIN_FOR_USER(nuser, callbacki->param[1]);
	SET_USER_FOR_APE(callbacki->g_ape, callbacki->param[1], nuser);

	init_user_info(callbacki->param[1], nuser, callbacki->g_ape);
	
	subuser_restor(getsubuser(callbacki->call_user, callbacki->host),
				   callbacki->g_ape);
	
	/*
	 * make own channel 
	 */
	CHANNEL *chan;
	char schan[MAX_CHAN_LEN];
	snprintf(schan, sizeof(schan), FRIEND_PIP_NAME"%s", callbacki->param[1]);
	if ((chan = getchan(schan, callbacki->g_ape)) == NULL) {
		chan = mkchan(schan, "Pipe For My Friend", callbacki->g_ape);
		if (chan == NULL) {
			smsalarm_msgf(callbacki->g_ape, "make channel %s failed", schan);
			SENDH(callbacki->fdclient, ERR_MAKE_CHAN, callbacki->g_ape);
			return (FOR_NOTHING);
		}
	}
	join(nuser, chan, callbacki->g_ape);

	HTBL_ITEM *item;
	HTBL *ulist = GET_USER_FRIEND_TBL(nuser);
	USERS *friend;
	CHANNEL *jchan;
	if (ulist != NULL)  {
		for (item = ulist->first; item != NULL; item = item->lnext) {
			/*
			 * invite my friends
			 */
			friend = GET_USER_FROM_APE(callbacki->g_ape, item->key);
			if (friend != NULL) {
				wlog_dbg("%s invite %s\n", callbacki->param[1], item->key);
				join(friend, chan, callbacki->g_ape);
			}
		
			/*
			 * join my friends
			 */
			snprintf(schan, sizeof(schan), FRIEND_PIP_NAME"%s", item->key);
			jchan = getchan(schan, callbacki->g_ape);
			if (jchan != NULL) {
				join(nuser, jchan, callbacki->g_ape);
			}
		}
	}

	set_json("sessid", nuser->sessid, &jlist);
	
	newraw = forge_raw(RAW_LOGIN, jlist);
	newraw->priority = 1;
	
	post_raw(newraw, nuser, callbacki->g_ape);

	//return (FOR_LOGIN | FOR_UPDATE_IP);

	send_raws(nuser->subuser, callbacki->g_ape);
	do_died(nuser->subuser);
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

	set_json("users", NULL, &jlist);

	if (ulist != NULL) {
		for (item = ulist->first; item != NULL; item = item->lnext) {
			user = (USERS*)item->addrs;
			json_attach(jlist, get_json_object_user(user), JSON_ARRAY);
		}
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
	char schan[MAX_CHAN_LEN];
	CHANNEL *chan;

	snprintf(schan, sizeof(schan), FRIEND_PIP_NAME"%s",
			 GET_UIN_FROM_USER(user));
	chan = getchan(schan, g_ape);
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

	left_all(user, g_ape);
	
	/* kill all users connections */
	
	clear_subusers(user);

	char *uin = GET_UIN_FROM_USER(user);
	if (uin != NULL && get_property(g_ape->properties, "userlist") != NULL) {
		hashtbl_erase(get_property(g_ape->properties, "userlist")->val, uin);
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

	wlog_dbg("prepare post %s", raw->data);
	
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
	wlog_dbg("%s done", raw->data);
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
	register_cmd("REGCLASS", 2, push_regpageclass, NEED_SESSID, g_ape);
	register_cmd("USERLIST", 1, push_userlist, NEED_SESSID, g_ape);
	register_cmd("FRIENDLIST", 1, push_friendlist, NEED_SESSID, g_ape);
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
