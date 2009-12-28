#include "plugins.h"
#include "config.h"
#include <mysql/mysql.h>

#include "mevent.h"
#include "data.h"
#include "mevent_uic.h"

#define MODULE_NAME "fkq"

#define FKQ_PIP_NAME "FangkequnPipe"
#define RAW_FKQDATA	"FKQDATA"

#define SET_SUBUSER_HOSTUIN(sub, uin)                                   \
    (add_property(&sub->properties, "hostuin", uin, EXTEND_STR, EXTEND_ISPRIVATE))

#define GET_SUBUSER_HOSTUIN(sub)                                    \
    (get_property(sub->properties, "hostuin") != NULL?              \
     (char*)get_property(sub->properties, "hostuin")->val: NULL)

#define SET_CHANNEL_PRIVATE(chan)                                       \
    (add_property(&chan->properties, "private", "1", EXTEND_STR, EXTEND_ISPRIVATE))

#define GET_CHANNEL_PRIVATE(chan)                                   \
    (get_property(chan->properties, "private",) != NULL?            \
     (char*)get_property(chan->properties, "private")->val: NULL)

static ace_plugin_infos infos_module = {
	"\"Fkq\" system", // Module Name
	"1.0",		       // Module Version
	"hunantv",         // Module Author
	"mod_fkq.conf"    // config file (bin/)
};

/*
 * file range 
 */
static bool user_has_fkq(char *uin)
{
	mevent_t *evt;
	struct data_cell *pc;
	
	int ret;

	if (!hn_isvaliduin(uin)) return false;
	
	evt = mevent_init_plugin("uic", REQ_CMD_MYSETTING, FLAGS_SYNC);
	mevent_add_u32(evt, NULL, "uin", atoi(uin));
	ret = mevent_trigger(evt);
	if (PROCESS_NOK(ret)) {
		wlog_err("get fkq for user %s failure %d", uin, ret);
        mevent_free(evt);
        return false;
	}

    pc = data_cell_search(evt->rcvdata, false, DATA_TYPE_U32, "fangkequn");
    if (pc != NULL && pc->v.ival == 1) {
        mevent_free(evt);
        return true;
    } else {
        mevent_free(evt);
        return false;
    }
}

/*
 * pro range
 */
static unsigned int fkq_join(callbackp *callbacki)
{
    char *hostUin;

    USERS *user = callbacki->call_user;
    CHANNEL *chan;

    JNEED_STR(callbacki->param, "hostUin", hostUin);
    
	if (!hn_isvaliduin(hostUin)) {
		return (RETURN_BAD_PARAMS);
	}
    
    if (user_has_fkq(hostUin)) {
        if ((chan = getchanf(callbacki->g_ape, FKQ_PIP_NAME"%s", hostUin))
            == NULL) {
            chan = mkchanf(callbacki->g_ape, FKQ_PIP_NAME"%s", hostUin);
            /*
             * don't set channel private here, so, every channel should be return by
             * session command with subuser_restore().
             * Client MUST filter these channels, popup hostuin's fangke user
             */
            //SET_CHANNEL_PRIVATE(chan);
        }
        if (chan != NULL) {
            join(user, chan, callbacki->g_ape);
            subuser *sub = getsubuser(user, callbacki->host);
            if (sub != NULL) {
                SET_SUBUSER_HOSTUIN(sub, hostUin);
            }
        }
    }
    
	return (RETURN_NOTHING);
}

static unsigned int fkq_send(callbackp *callbacki)
{
	json_item *jlist = NULL;
	char *msg, *pipe;
	
	APE_PARAMS_INIT();
	
	if ((msg = JSTR(msg)) != NULL && (pipe = JSTR(pipe)) != NULL) {
		jlist = json_new_object();
		
		json_set_property_strZ(jlist, "msg", msg);

#if 0
        json_set_property_strZ(jlist, "aaa", "bbbbb");
        
        json_item *jcopy = json_item_copy(jlist, NULL);
        json_set_property_strZ(jcopy, "hello", "world");
        struct jsontring *str;
        str = json_to_string(jcopy, NULL, 0);
        wlog_dbg("jcopy str %s", str->jstring);

        
        json_item *jchild = json_new_object();
		json_set_property_strZ(jchild, "child", "c1");

        json_item *jgson = json_new_object();
		json_set_property_strZ(jgson, "gson", "gson1");
        json_set_property_objZ(jchild, "gsons", jgson);
        
        json_set_property_objZ(jlist, "childs", jchild);
#endif

		post_to_pipe(jlist, RAW_FKQDATA, pipe, callbacki->call_subuser,
                     callbacki->g_ape);
		
		return (RETURN_NOTHING);
	}
	
	return (RETURN_BAD_PARAMS);
}

static void fkq_event_delsubuser(subuser *del, acetables *g_ape)
{
    char *uin = GET_SUBUSER_HOSTUIN(del);
    if (uin != NULL) {
        USERS *user = del->user;
        subuser *cur = user->subuser;
        char *otheruin;
        bool lastone = true;
        while (cur != NULL) {
            otheruin = GET_SUBUSER_HOSTUIN(cur);
            if (cur != del && otheruin != NULL && !strcmp(uin, otheruin)) {
                lastone = false;
                break;
            }
            cur = cur->next;
        }
            
        if (lastone) {
            CHANNEL *chan = getchanf(g_ape, FKQ_PIP_NAME"%s", uin);
            if (chan != NULL) {
                left(del->user, chan, g_ape);
            }
        }
    }
    
	clear_properties(&del->properties);
	
	destroy_raw_pool(del->raw_pools.low.rawhead);
	destroy_raw_pool(del->raw_pools.high.rawhead);
    del->raw_pools.low.rawhead = NULL;
    del->raw_pools.low.rawfoot = NULL;
    del->raw_pools.high.rawhead = NULL;
    del->raw_pools.high.rawfoot = NULL;

	if (del->state == ALIVE) {
		del->wait_for_free = 1;
		do_died(del);
	} else {
		free(del);
	}
} 

static void init_module(acetables *g_ape)
{
    st_fkq *stdata = xmalloc(sizeof(st_fkq));
    add_property(&g_ape->properties, "msgstatic", stdata,
                 EXTEND_POINTER, EXTEND_ISPRIVATE);
	register_cmd("FKQ_JOIN", fkq_join, NEED_SESSID, g_ape);
	register_cmd("FKQ_SEND", fkq_send, NEED_SESSID, g_ape);
}

static ace_callbacks callbacks = {
	NULL,				/* Called when new user is added */
	NULL,				/* Called when a user is disconnected */
    fkq_event_delsubuser,     /* Called when a subuser is disconnected */
	NULL,				/* Called when new chan is created */
    NULL,               /* Called when a chan removed */
	NULL,				/* Called when a user join a channel */
	NULL,				/* Called when a user leave a channel */
	NULL,				/* Called at each tick, passing a subuser */
	NULL				/* Called when a subuser receiv a message */
};

APE_INIT_PLUGIN(MODULE_NAME, init_module, callbacks)
