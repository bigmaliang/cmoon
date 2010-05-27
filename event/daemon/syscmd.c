#include "syscmd.h"
#include "ClearSilver.h"

int sys_cmd_cache_get(struct queue_entry *q, struct cache *cd, bool reply)
{
    unsigned char *val = NULL;
    size_t vsize = 0;
    char *key;
    int hit, ret;

    if (q == NULL || cd == NULL) {
        ret = REP_ERR;
        goto done;
    }

	key = hdf_get_value(q->hdfrcv, VNAME_CACHE_KEY, NULL);
    if (!key) {
        ret = REP_ERR_BADPARAM;
        goto done;
    }
    hit = cache_get(cd, (unsigned char*)key, strlen((char*)key), &val, &vsize);
    if (hit)
        ret = REP_OK;
	else
        ret = REP_ERR_CACHE_MISS;

 done:
    if (reply) {
        if (ret == REP_OK) {
            q->req->reply_long(q->req, reply, val, vsize);
        } else {
            q->req->reply_mini(q->req, reply);
        }
    } else {
        if (ret == REP_OK && val != NULL && vsize > 0) {
            /* if we don't reply to client, store them in replydata */
			hdf_set_value(q->hdfsnd, VNAME_CACHE_VAL, (char*)val);
        }
    }

    return ret;
}

int sys_cmd_cache_set(struct queue_entry *q, struct cache *cd, bool reply)
{
    char *val = NULL;
    size_t vsize = 0;
    char *key;
    int ret;

    if (q == NULL || cd == NULL) {
        ret = REP_ERR;
        goto done;
    }

	key = hdf_get_value(q->hdfrcv, VNAME_CACHE_KEY, NULL);
    if (!key) {
        ret = REP_ERR_BADPARAM;
        goto done;
    }

	HDF *node = hdf_get_obj(q->hdfrcv, VNAME_CACHE_VAL);
    if (!node) {
        ret = REP_ERR_BADPARAM;
        goto done;
    }
	
	val = hdf_obj_value(node);
	vsize = strlen(val)+1;
    cache_set(cd, (unsigned char*)key, strlen((char*)key),
			  (unsigned char*)val, vsize);

    ret = REP_OK;
    
 done:
    if (reply) {
        /* nothing to be returned on set, except set status */
        q->req->reply_mini(q->req, reply);
    }

    return ret;
}

int sys_cmd_cache_del(struct queue_entry *q, struct cache *cd, bool reply)
{
    char *key;
    int ret;

    if (q == NULL || cd == NULL) {
        ret = REP_ERR;
        goto done;
    }

	key = hdf_get_value(q->hdfrcv, VNAME_CACHE_KEY, NULL);
    if (!key) {
        ret = REP_ERR_BADPARAM;
        goto done;
    }
    cache_del(cd, (unsigned char*)key, strlen(key));

    ret = REP_OK;
    
 done:
    if (reply) {
        q->req->reply_mini(q->req, reply);
    }

    return ret;
}

int sys_cmd_cache_empty(struct queue_entry *q, struct cache **cd, bool reply)
{
    int ret;
    size_t num;

    if (q == NULL || cd == NULL || *cd == NULL) {
        ret = REP_ERR;
        goto done;
    }

    
    struct cache *oldcd = *cd;
    
    num = oldcd->numobjs;
    if (oldcd) cache_free(oldcd);

    *cd = cache_create(num, 0);
    
    ret = REP_OK;
    
 done:
    if (reply) {
        q->req->reply_mini(q->req, reply);
    }

    return ret;
}
