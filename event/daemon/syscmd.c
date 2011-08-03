#include "syscmd.h"
#include "ClearSilver.h"

NEOERR* sys_cmd_cache_get(struct queue_entry *q, struct cache *cd, bool reply)
{
    unsigned char *val = NULL;
    size_t vsize = 0;
    char *key;
    NEOERR *err = STATUS_OK;

    if (q == NULL || cd == NULL) {
        err = nerr_raise(REP_ERR, "param null");
        goto done;
    }

    key = hdf_get_value(q->hdfrcv, VNAME_CACHE_KEY, NULL);
    if (!key) {
        err = nerr_raise(REP_ERR_BADPARAM, "need %s", VNAME_CACHE_KEY);
        goto done;
    }

    if (!cache_get(cd, (unsigned char*)key, strlen((char*)key), &val, &vsize))
        err = nerr_raise(REP_ERR_CACHE_MISS, "miss %s", key);

 done:
    if (reply) {
        if (err == STATUS_OK) {
            q->req->reply_long(q->req, reply, val, vsize);
        } else {
            q->req->reply_mini(q->req, reply);
        }
    } else {
        if (err == STATUS_OK && val != NULL && vsize > 0) {
            /* if we don't reply to client, store them in replydata */
            hdf_set_value(q->hdfsnd, VNAME_CACHE_VAL, (char*)val);
        }
    }

    return err;
}

NEOERR* sys_cmd_cache_set(struct queue_entry *q, struct cache *cd, bool reply)
{
    char *val = NULL;
    size_t vsize = 0;
    char *key;
    NEOERR *err = STATUS_OK;

    if (q == NULL || cd == NULL) {
        err = nerr_raise(REP_ERR, "param null");
        goto done;
    }

    key = hdf_get_value(q->hdfrcv, VNAME_CACHE_KEY, NULL);
    if (!key) {
        err = nerr_raise(REP_ERR_BADPARAM, "need %s", VNAME_CACHE_KEY);
        goto done;
    }

    HDF *node = hdf_get_obj(q->hdfrcv, VNAME_CACHE_VAL);
    if (!node) {
        err = nerr_raise(REP_ERR_BADPARAM, "need %s", VNAME_CACHE_VAL);
        goto done;
    }
    
    val = hdf_obj_value(node);
    vsize = strlen(val)+1;
    cache_set(cd, (unsigned char*)key, strlen((char*)key),
              (unsigned char*)val, vsize, 0);

 done:
    if (reply) {
        /* nothing to be returned on set, except set status */
        q->req->reply_mini(q->req, reply);
    }

    return err;
}

NEOERR* sys_cmd_cache_del(struct queue_entry *q, struct cache *cd, bool reply)
{
    char *key;
    NEOERR *err = STATUS_OK;

    if (q == NULL || cd == NULL) {
        err = nerr_raise(REP_ERR, "param null");
        goto done;
    }

    key = hdf_get_value(q->hdfrcv, VNAME_CACHE_KEY, NULL);
    if (!key) {
        err = nerr_raise(REP_ERR_BADPARAM, "need %s", VNAME_CACHE_KEY);
        goto done;
    }
    cache_del(cd, (unsigned char*)key, strlen(key));

 done:
    if (reply) {
        q->req->reply_mini(q->req, reply);
    }

    return err;
}

NEOERR* sys_cmd_cache_empty(struct queue_entry *q, struct cache **cd, bool reply)
{
    NEOERR *err = STATUS_OK;
    size_t num;

    if (q == NULL || cd == NULL || *cd == NULL) {
        err = nerr_raise(REP_ERR, "param null");
        goto done;
    }

    
    struct cache *oldcd = *cd;
    
    num = oldcd->numobjs;
    if (oldcd) cache_free(oldcd);

    *cd = cache_create(num, 0);
    
 done:
    if (reply) {
        q->req->reply_mini(q->req, reply);
    }

    return err;
}
