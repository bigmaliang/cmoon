#include "apev.h"

/*
 * mostly, we have ONLY_ONE backend for apev/x, so, we use a list instead of hash
 * one single compare can be 3 ~ 4 faster than hash_lookup
 */
/*static HASH *etbl = NULL;*/
struct event_entry *etbl = NULL;

static NEOERR* parse_event(struct req_info *req)
{
    unsigned char *ename;
    uint32_t esize, rsize;
    unsigned char *pos;
    HDF *hdfrcv = NULL;
    NEOERR *err, *neede;
    int retcode;

    if (!etbl) return nerr_raise(NERR_ASSERT, "v backend hasn't init");
    
    /*
     * Request format:
     * 4        esize
     * esize    ename
     *
     * 4        vtype
     * 4        ksize
     * ksize    key
     * 4        vsize/ival
     * vsize    val
     *
     */
    pos = (unsigned char*)req->payload;
    esize = * (uint32_t *) pos; esize = ntohl(esize);

    pos = pos + sizeof(uint32_t);
    ename = pos;
    *(pos+esize) = '\0';

    pos = pos + esize;
    rsize = unpack_hdf(pos, req->psize-esize-sizeof(uint32_t), &hdfrcv);
    if (rsize == 0 || rsize+esize+sizeof(uint32_t) > MAX_PACKET_LEN ||
        req->psize < esize) {
        //stats.net_broken_req++;
        return nerr_raise(NERR_PARSE, "unpack_hdf faied with %d", rsize);
    }
    
    struct event_entry *e = etbl;
    while (e) {
        if (!strcmp(e->name, (char*)ename)) {
            struct queue_entry q;

            memset(&q, sizeof(q), 0x0);

            q.operation = (uint32_t)req->cmd;
            q.ename = ename;
            q.esize = esize;
            q.hdfrcv = hdfrcv;
            hdf_init(&(q.hdfsnd));
            q.req = req;
        
            err = e->process_driver(e, &q);

            /*
             * set errorxxx to q.hdfsnd
             */
            neede = err;
            while (neede && neede != INTERNAL_ERR) {
                if (neede->error != NERR_PASS) break;
                neede = neede->next;
            }
            retcode = neede ? neede->error: REP_OK;
            if (PROCESS_NOK(retcode)) {
                STRING s; string_init(&s);
                nerr_error_traceback(err, &s);
                hdf_set_value(q.hdfsnd, "errtrace", s.buf);
                string_clear(&s);
                hdf_set_int_value(q.hdfsnd, "errcode", neede->error);
            }

            /*
             * send q.hdfsnd to client if present
             */
            if (hdf_obj_child(q.hdfsnd) != NULL) {
                unsigned char *buf = calloc(1, MAX_PACKET_LEN);
                if (!buf) goto done;

                size_t vsize;
                vsize = pack_hdf(q.hdfsnd, buf, MAX_PACKET_LEN);
                if (vsize == 0) goto done;
 
                q.req->reply_long(q.req, retcode, buf, vsize);

                free(buf);
            }
        
        done:
            hdf_destroy(&(q.hdfrcv));
            hdf_destroy(&(q.hdfsnd));

            /*
             * return error to caller for trace
             */
            return nerr_pass(err);
        }

        e = e->next;
    }

    return nerr_raise(NERR_NOT_FOUND, "%s backend not found", ename);
}

NEOERR* parse_regist_v(struct event_entry *e)
{
    NEOERR *err;
    
    if (!e || !e->name) return nerr_raise(NERR_ASSERT, "input error");

    err = e->start_driver();
    if (err != STATUS_OK) return nerr_pass(err);
    
    e->next = etbl;
    etbl = e;

    return STATUS_OK;
}


/* Parse an incoming message. Note that the protocol might have sent this
 * directly over the network (ie. TIPC) or might have wrapped it around (ie.
 * TCP). Here we only deal with the clean, stripped, non protocol-specific
 * message. */
NEOERR* parse_message(struct req_info *req, const unsigned char *buf, size_t len)
{
    uint32_t hdr, ver, id;
    uint16_t cmd, flags;
    const unsigned char *payload;
    size_t psize;

    if (len < 17) {
        //stats.net_broken_req++;
        //req->reply_mini(req, REP_ERR_BROKEN);
        return nerr_raise(NERR_ASSERT, "packet broken %d", len);
    }

    MSG_DUMP("recv: ",  buf, len);
    
    /* The header is:
     * 4 bytes    Version + ID
     * 2 bytes    Command
     * 2 bytes    Flags
     * Variable     Payload
     */

    hdr = * ((uint32_t *) buf);
    hdr = htonl(hdr);

    /* FIXME: little endian-only */
    ver = (hdr & 0xF0000000) >> 28;
    id = hdr & 0x0FFFFFFF;
    req->id = id;

    cmd = ntohs(* ((uint16_t *) buf + 2));
    flags = ntohs(* ((uint16_t *) buf + 3));

    if (ver != PROTO_VER) {
        //stats.net_version_mismatch++;
        //req->reply_mini(req, REP_ERR_VER);
        return nerr_raise(NERR_ASSERT, "version mismatch %d", ver);
    }

    /* We define payload as the stuff after buf. But be careful because
     * there might be none (if len == 1). Doing the pointer arithmetic
     * isn't problematic, but accessing the payload should be done only if
     * we know we have enough data. */
    payload = buf + 8;
    psize = len - 8;

    /* Store the id encoded in network byte order, so that we don't have
     * to calculate it at send time. */
    req->id = htonl(id);
    req->cmd = cmd;
    req->flags = flags;
    req->payload = payload;
    req->psize = psize;

    return nerr_pass(parse_event(req));
}
