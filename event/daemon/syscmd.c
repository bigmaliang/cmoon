#include "syscmd.h"

int sys_cmd_cache_get(struct queue_entry *q, struct cache *cd, bool reply)
{
    struct data_cell *c;
    unsigned char *val = NULL;
    size_t vsize = 0;
    unsigned char *key;
    int hit, ret;

    if (q == NULL || cd == NULL) {
        ret = REP_ERR;
        goto done;
    }
    
    c = data_cell_search(q->dataset, false,
                         DATA_TYPE_STRING, VNAME_CACHE_KEY);
    if (c == NULL) {
        ret = REP_ERR_BADPARAM;
        goto done;
    }
    key = c->v.sval.val;
    hit = cache_get(cd, key, strlen((char*)key), &val, &vsize);
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
            if (!q->replydata) {
                unpack_data("root", val, vsize, &q->replydata);
            } else {
                unpack_data("return", val, vsize, &c);
                data_cell_append(q->replydata, c);
            }
        }
    }

    return ret;
}

int sys_cmd_cache_set(struct queue_entry *q, struct cache *cd, bool reply)
{
    struct data_cell *c;
    unsigned char *val = NULL;
    size_t vsize = 0;
    unsigned char *key;
    int ret;

    if (q == NULL || cd == NULL) {
        ret = REP_ERR;
        goto done;
    }
    
    c = data_cell_search(q->dataset, false,
                         DATA_TYPE_STRING, VNAME_CACHE_KEY);
    if (c == NULL) {
        ret = REP_ERR_BADPARAM;
        goto done;
    }
    key = c->v.sval.val;
    c = data_cell_search(q->dataset, false,
                         DATA_TYPE_ANY, VNAME_CACHE_VAL);
    if (c == NULL) {
        ret = REP_ERR_BADPARAM;
        goto done;
    }
    val = calloc(1, MAX_PACKET_LEN);
    if (val == NULL) {
        ret = REP_ERR_MEM;
        goto done;
    }
    vsize = pack_data_any(NULL, c, val, MAX_PACKET_LEN - RESERVE_SIZE);
    if (vsize == 0) {
        ret = REP_ERR_PACK;
        goto done;
    }
    * (uint32_t *) (val+vsize) = htonl(DATA_TYPE_EOF);
    vsize += sizeof(uint32_t);
    cache_set(cd, key, strlen((char*)key), val, vsize);

    ret = REP_OK;
    
 done:
    if (reply) {
        /* nothing to be returned on set, except set status */
        q->req->reply_mini(q->req, reply);
    }

    if (val) free(val);
    
    return ret;
}

int sys_cmd_cache_del(struct queue_entry *q, struct cache *cd, bool reply)
{
    struct data_cell *c;
    unsigned char *key;
    int ret;

    if (q == NULL || cd == NULL) {
        ret = REP_ERR;
        goto done;
    }
    
    c = data_cell_search(q->dataset, false,
                         DATA_TYPE_STRING, VNAME_CACHE_KEY);
    if (c == NULL) {
        ret = REP_ERR_BADPARAM;
        goto done;
    }
    key = c->v.sval.val;
    cache_del(cd, key, strlen((char*)key));

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
