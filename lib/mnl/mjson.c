#include "mheads.h"

NEOERR* mjson_import_from_hdf(HDF *hdf, struct json_object **out)
{
    if (hdf == NULL) return nerr_raise(NERR_ASSERT, "paramter null");

    struct json_object *jret, *jso;
    char *val, *type;
    bool array = false;

    type = mcs_obj_attr(hdf, "type");
    if (type && !strcmp(type, "array")) {
        array = true;
        jret = json_object_new_array();
    } else {
        jret = json_object_new_object();
    }

    hdf = hdf_obj_child(hdf);

    while (hdf) {
        jso = NULL;
        
        if ((val = hdf_obj_value(hdf)) != NULL) {
            type = mcs_obj_attr(hdf, "type");
            if (type != NULL && !strcmp(type, "int")) {
                jso = json_object_new_int(atoi(val));
            } else {
                jso = json_object_new_string(val);
            }
            if (array)
                json_object_array_add(jret, jso);
            else
                json_object_object_add(jret, hdf_obj_name(hdf), jso);
        }

        if (hdf_obj_child(hdf) != NULL) {
            mjson_import_from_hdf(hdf, &jso);
            if (array)
                json_object_array_add(jret, jso);
            else
                json_object_object_add(jret, hdf_obj_name(hdf), jso);
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

NEOERR* mjson_export_to_hdf(HDF *node, struct json_object *o, bool drop)
{
    if (!node) return nerr_raise(NERR_ASSERT, "paramter null");
    
    char *s = hdf_obj_value(node);
    
    struct json_object *obj;
    struct array_list *list;
    enum json_type type;
    NEOERR *err;
    HDF *cnode;
    char tok[64];
    int i;
    
    char *key; struct json_object *val; struct lh_entry *entry;
    
    obj = o;

    if (!obj && s && *s) {
        obj = json_tokener_parse(s);
    }
    if (!obj || (int)obj < 0) return nerr_raise(NERR_ASSERT, "json object null");;

    type = json_object_get_type(obj);

    switch (type) {
    case json_type_boolean:
        hdf_set_int_value(node, NULL, json_object_get_boolean(obj));
        break;
    case json_type_int:
        hdf_set_int_value(node, NULL, json_object_get_int(obj));
        break;
    case json_type_double:
        sprintf(tok, "%f", json_object_get_double(obj));
        hdf_set_value(node, NULL, tok);
        break;
    case json_type_string:
        hdf_set_value(node, NULL, json_object_get_string(obj));
        break;
    case json_type_array:
        list = json_object_get_array(obj);
        for (i = 0; i < list->length; i++) {
            sprintf(tok, "%d", i);
            hdf_get_node(node, tok, &cnode);
            err = mjson_export_to_hdf(cnode, (struct json_object*)list->array[i], true);
            if (err != STATUS_OK) return nerr_pass(err);
        }
        break;
    case json_type_object:
        for(entry = json_object_get_object(obj)->head;
            (entry ? (key = (char*)entry->k,
                      val = (struct json_object*)entry->v, entry) : 0);
            entry = entry->next) {
            hdf_get_node(node, key, &cnode);
            err = mjson_export_to_hdf(cnode, val, true);
            if (err != STATUS_OK) return nerr_pass(err);
        }
        break;
    default:
        break;
    }

    if (!o) json_object_put(obj);
    if (drop && o) json_object_put(o);

    return STATUS_OK;
}
