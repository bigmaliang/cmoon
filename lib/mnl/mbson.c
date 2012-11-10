#include "mheads.h"

static void json_append_to_bson(bson* b, char *key, struct json_object *val)
{
    if (!b || !key || !val) return;

    struct array_list *list;
    enum json_type type;
    bson *sub;
    char tok[64];

    type = json_object_get_type(val);

    switch (type) {
    case json_type_boolean:
        bson_append_boolean(b, key, json_object_get_boolean(val));
        break;
    case json_type_int:
        bson_append_int32(b, key, json_object_get_int(val));
        break;
    case json_type_double:
        bson_append_double(b, key, json_object_get_double(val));
        break;
    case json_type_string:
        bson_append_string(b, key, json_object_get_string(val), -1);
        break;
    case json_type_array:
        sub = bson_new();

        list = json_object_get_array(val);
        for (int pos = 0; pos < list->length; pos++) {
            sprintf(tok, "%d", pos);
            json_append_to_bson(sub, tok, (struct json_object*)list->array[pos]);
        }
        
        bson_finish(sub);
        bson_append_array(b, key, sub);
        bson_free(sub);
        break;
    case json_type_object:
        sub = mbson_new_from_jsonobj(val, true);
        bson_append_document(b, key, sub);
        bson_free(sub);
        break;
    default:
        break;
    }
    
    
}

bson* mbson_new_from_jsonobj(struct json_object *obj, bool finish)
{
    char *key; struct json_object *val; struct lh_entry *entry;
    bson *bson;
    
    if (!obj) return NULL;

    if (json_object_get_type(obj) != json_type_object) {
        json_object_put(obj);
        return NULL;
    }
    
    bson = bson_new();

    for (entry = json_object_get_object(obj)->head;
         (entry ? (key = (char*)entry->k,
                   val = (struct json_object*)entry->v, entry) : 0);
         entry = entry->next) {
        json_append_to_bson(bson, key, val);
    }
    
    if (finish) bson_finish(bson);

    json_object_put(obj);

    return bson;
}

bson* mbson_new_from_string(const char *s, bool finish)
{
    if (!s) return NULL;

    struct json_object *obj;

    obj = json_tokener_parse(s);
    if (!obj) return NULL;

    return mbson_new_from_jsonobj(obj, finish);
}

struct json_object* mbson_export_to_jsonobj(bson *doc, bool array)
{
    if (!doc) return NULL;

    struct json_object *jret, *jso;
    bson_cursor *c;
    bson_type type;
    char *key;
    double vd;
    bson *vb;
    char *vs;
    int vi;
    gint64 vi6;
    bool vbl;
    

    c = bson_cursor_new(doc);

    if (array)
        jret = json_object_new_array();
    else
        jret = json_object_new_object();
    
    while (bson_cursor_next(c)) {
        key = (char*)bson_cursor_key(c);
        type = bson_cursor_type(c);
        jso = NULL;
        
        switch (type) {
        case BSON_TYPE_DOUBLE:
            bson_cursor_get_double(c, &vd);
            jso = json_object_new_double(vd);
            break;
        case BSON_TYPE_STRING:
            bson_cursor_get_string(c, (const gchar**)&vs);
            jso = json_object_new_string(vs);
            break;
        case BSON_TYPE_DOCUMENT:
            bson_cursor_get_document(c, &vb);
            jso = mbson_export_to_jsonobj(vb, false);
            break;
        case BSON_TYPE_ARRAY:
            bson_cursor_get_array(c, &vb);
            jso = mbson_export_to_jsonobj(vb, true);
            break;
        case BSON_TYPE_BOOLEAN:
            bson_cursor_get_boolean(c, (gboolean*)&vbl);
            jso = json_object_new_boolean(vbl);
            break;
        case BSON_TYPE_INT32:
            bson_cursor_get_int32(c, &vi);
            jso = json_object_new_int(vi);
            break;
        case BSON_TYPE_UTC_DATETIME:
        case BSON_TYPE_TIMESTAMP:
        case BSON_TYPE_INT64:
            /* TODO UTC, TIME bug? */
            bson_cursor_get_int64(c, &vi6);
            jso = json_object_new_int64(vi6);
            break;
        default:
            break;
        }

        if (jso) {
            if (array)
                json_object_array_add(jret, jso);
            else
                json_object_object_add(jret, key, jso);
        }
    }

    bson_cursor_free(c);

    return jret;
}

char* mbson_string(bson *doc)
{
    struct json_object *obj;
    char *s;
    
    obj = mbson_export_to_jsonobj(doc, false);
    if (!obj) return NULL;

    s = strdup(json_object_to_json_string(obj));

    json_object_put(obj);

    return s;
}

NEOERR* mbson_import_from_hdf(HDF *node, bson **out, bool finish)
{
    if (!node || !out) return nerr_raise(NERR_ASSERT, "paramter null");

    char *key, *val;
    CnodeType type;
    bson *doc, *sub;

    doc = bson_new();

    node = hdf_obj_child(node);
    while (node) {
        key = hdf_obj_name(node);
        val = hdf_obj_value(node);
        type = mcs_get_int_attr(node, NULL, "type", CNODE_TYPE_STRING);
            
        if (hdf_obj_child(node) != NULL) {
            mbson_import_from_hdf(node, &sub, true);
            if (type == CNODE_TYPE_ARRAY)
                bson_append_array(doc, key, sub);
            else
                bson_append_document(doc, key, sub);
            bson_free(sub);
        } else if (val) {
            switch (type) {
            case CNODE_TYPE_BOOL:
                bson_append_boolean(doc, key, (bool)atoi(val));
                break;
            case CNODE_TYPE_INT:
                bson_append_int32(doc, key, atoi(val));
                break;
            case CNODE_TYPE_FLOAT:
                bson_append_double(doc, key, atof(val));
                break;
            case CNODE_TYPE_INT64:
            case CNODE_TYPE_DATETIME:
            case CNODE_TYPE_TIMESTAMP:
                bson_append_int64(doc, key, mcs_get_int64_value(node, NULL, 0));
                break;
            default:
                bson_append_string(doc, key, val, -1);
                break;
            }
        }
        
        node = hdf_obj_next(node);
    }

    if (finish) bson_finish(doc);

    *out = doc;
    
    return STATUS_OK;
}

NEOERR* mbson_export_to_hdf(HDF *node, bson *doc, bool array)
{
    if (!node || !doc) return nerr_raise(NERR_ASSERT, "paramter null");

    bson_cursor *c;
    bson_type type;
    char *key;
    HDF *cnode;
    double vd;
    bson *vb;
    char *vs;
    int vi;
    gint64 vi6;
    bool vbl;
    NEOERR *err;

    c = bson_cursor_new(doc);

    while (bson_cursor_next(c)) {
        key = (char*)bson_cursor_key(c);
        type = bson_cursor_type(c);

        switch (type) {
        case BSON_TYPE_DOUBLE:
            bson_cursor_get_double(c, &vd);
            MCS_SET_FLOAT_VALUE_WITH_TYPE(node, key, vd, CNODE_TYPE_FLOAT);
            break;
        case BSON_TYPE_STRING:
            bson_cursor_get_string(c, (const gchar**)&vs);
            MCS_SET_VALUE_WITH_TYPE(node, key, vs, CNODE_TYPE_STRING);
            break;
        case BSON_TYPE_DOCUMENT:
            bson_cursor_get_document(c, &vb);
            hdf_get_node(node, key, &cnode);
            hdf_set_value(cnode, NULL, "foo");
            MCS_SET_INT_ATTR(cnode, NULL, "type", CNODE_TYPE_OBJECT);
            err = mbson_export_to_hdf(cnode, vb, false);
            if (err != STATUS_OK) return nerr_pass(err);
            break;
        case BSON_TYPE_ARRAY:
            bson_cursor_get_array(c, &vb);
            hdf_get_node(node, key, &cnode);
            hdf_set_value(cnode, NULL, "foo");
            MCS_SET_INT_ATTR(cnode, NULL, "type", CNODE_TYPE_ARRAY);
            err = mbson_export_to_hdf(cnode, vb, true);
            if (err != STATUS_OK) return nerr_pass(err);
            break;
        case BSON_TYPE_BOOLEAN:
            bson_cursor_get_boolean(c, (gboolean*)&vbl);
            MCS_SET_INT_VALUE_WITH_TYPE(node, key, vbl, CNODE_TYPE_BOOL);
            break;
        case BSON_TYPE_INT32:
            bson_cursor_get_int32(c, &vi);
            MCS_SET_INT_VALUE_WITH_TYPE(node, key, vi, CNODE_TYPE_INT);
            break;
        case BSON_TYPE_UTC_DATETIME:
        case BSON_TYPE_TIMESTAMP:
        case BSON_TYPE_INT64:
            /* TODO UTC, TIME bug? */
            bson_cursor_get_int64(c, &vi6);
            MCS_SET_INT64_VALUE_WITH_TYPE(node, key, vi6, CNODE_TYPE_INT64);
            break;
        default:
            break;
        }
    }

    bson_cursor_free(c);
    
    return STATUS_OK;
}
