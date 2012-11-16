#include "mheads.h"

NEOERR* mjson_import_from_hdf(HDF *hdf, struct json_object **out)
{
    if (hdf == NULL) return nerr_raise(NERR_ASSERT, "paramter null");

    struct json_object *jret, *jso;
    char *val;
    CnodeType ptype, ctype;
    
    MCS_GET_INT_ATTR(hdf, NULL, "type", CNODE_TYPE_STRING, ptype);
    if (ptype == CNODE_TYPE_ARRAY) {
        jret = json_object_new_array();
    } else {
        jret = json_object_new_object();
    }

    hdf = hdf_obj_child(hdf);
    
    while (hdf) {
        jso = NULL;
        
        if (hdf_obj_child(hdf) != NULL) {
            mjson_import_from_hdf(hdf, &jso);
            
            if (ptype == CNODE_TYPE_ARRAY) json_object_array_add(jret, jso);
            else json_object_object_add(jret, hdf_obj_name(hdf), jso);
            
        } else if ((val = hdf_obj_value(hdf)) != NULL) {
            MCS_GET_INT_ATTR(hdf, NULL, "type", CNODE_TYPE_STRING, ctype);
            if (ctype == CNODE_TYPE_INT) {
                jso = json_object_new_int(atoi(val));
            } else {
                jso = json_object_new_string(val);
            }
        
            if (ptype == CNODE_TYPE_ARRAY) json_object_array_add(jret, jso);
            else json_object_object_add(jret, hdf_obj_name(hdf), jso);

        }
        
        hdf = hdf_obj_next(hdf);
    }
    
    *out = jret;
    
    return STATUS_OK;
}

void mjson_output_hdf(HDF *hdf, time_t second)
{
    if (second > 0) {
        mhttp_cache_headers(second);
    }
    
    NEOERR *err = cgiwrap_writef("Content-Type: text/html; charset=UTF-8\r\n\r\n");
    TRACE_NOK(err);

    HDF *ohdf = hdf_get_obj(hdf, PRE_OUTPUT);
    if (!ohdf) return;

    struct json_object *out = NULL;
    err = mjson_import_from_hdf(ohdf, &out);
    TRACE_NOK(err);

    err = cgiwrap_writef("%s\n", json_object_to_json_string(out));
    TRACE_NOK(err);

    json_object_put(out);
}

void mjson_execute_hdf(HDF *hdf, char *cb, time_t second)
{
    if (second > 0) {
        mhttp_cache_headers(second);
    }
    
    NEOERR *err = cgiwrap_writef("Content-Type: text/html; charset=UTF-8\r\n\r\n");
    TRACE_NOK(err);

    HDF *ohdf = hdf_get_obj(hdf, PRE_OUTPUT);
    if (!ohdf) return;

    struct json_object *out = NULL;
    err = mjson_import_from_hdf(ohdf, &out);
    TRACE_NOK(err);

    cgiwrap_writef("%s(%s);\n", cb, json_object_to_json_string(out));
    TRACE_NOK(err);

    json_object_put(out);
}

static inline void json_append_to_hdf(HDF *node, char *key, struct json_object *obj, int flag)
{
    if (!node || !key || !obj) return;

    struct array_list *list;
    enum json_type type;
    char tok[64];
    HDF *cnode;
    
    type = json_object_get_type(obj);

    switch (type) {
    case json_type_boolean:
        hdf_set_int_value(node, key, json_object_get_boolean(obj));
        if (flag & MJSON_EXPORT_TYPE) MCS_SET_INT_ATTR(node, key, "type", CNODE_TYPE_BOOL);
        break;
    case json_type_int:
        hdf_set_int_value(node, key, json_object_get_int(obj));
        if (flag & MJSON_EXPORT_TYPE) MCS_SET_INT_ATTR(node, key, "type", CNODE_TYPE_INT);
        break;
    case json_type_double:
        mcs_set_float_value(node, key, json_object_get_double(obj));
        if (flag & MJSON_EXPORT_TYPE)
            MCS_SET_INT_ATTR(node, key, "type", CNODE_TYPE_FLOAT);
        break;
    case json_type_string:
        hdf_set_value(node, key, json_object_get_string(obj));
        if (flag & MJSON_EXPORT_TYPE)
            MCS_SET_INT_ATTR(node, key, "type", CNODE_TYPE_STRING);
        break;
    case json_type_array:
        hdf_get_node(node, key, &cnode);
        list = json_object_get_array(obj);
        for (int i = 0; i < list->length; i++) {
            sprintf(tok, "%d", i);
            json_append_to_hdf(cnode, tok, (struct json_object*)list->array[i], flag);
        }
        if (flag & MJSON_EXPORT_TYPE)
            MCS_SET_INT_ATTRR(cnode, NULL, "type", CNODE_TYPE_ARRAY);
        break;
    case json_type_object:
        hdf_get_node(node, key, &cnode);
        json_object_object_foreach(obj, key, val) {
            json_append_to_hdf(cnode, key, val, flag);
        }
        if (flag & MJSON_EXPORT_TYPE)
            MCS_SET_INT_ATTR(cnode, NULL, "type", CNODE_TYPE_OBJECT);
        break;
    default:
        break;
    }
}

NEOERR* mjson_export_to_hdf(HDF *node, struct json_object *obj, int flag, bool drop)
{
    //if (!obj || (int)obj < 0) return nerr_raise(NERR_ASSERT, "json object null");;
    if (!node || !obj) return nerr_raise(NERR_ASSERT, "paramter null");

    if (json_object_get_type(obj) != json_type_object) {
        if (drop) json_object_put(obj);
        return nerr_raise(NERR_ASSERT, "not a json object");
    }

    json_object_object_foreach(obj, key, val) {
        json_append_to_hdf(node, key, val, flag);
    }
    
    if (drop) json_object_put(obj);

    return STATUS_OK;
}

NEOERR* mjson_string_to_hdf(HDF *node, char *str, int flag)
{
    if (!node) return nerr_raise(NERR_ASSERT, "paramter null");

    if (!str) str = hdf_obj_value(node);

    struct json_object *obj;

    obj = json_tokener_parse(str);

    if (!obj) return nerr_raise(NERR_ASSERT, "json object null");

    return mjson_export_to_hdf(node, obj, flag, true);
}
