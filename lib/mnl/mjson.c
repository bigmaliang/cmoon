#include "mheads.h"

void mjson_asm_objs(HDF *hdf, struct json_object **out)
{
    if (hdf == NULL)
        return;

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
            mjson_asm_objs(hdf, &jso);
            if (array)
                json_object_array_add(jret, jso);
            else
                json_object_object_add(jret, hdf_obj_name(hdf), jso);
        }
        
        hdf = hdf_obj_next(hdf);
    }

    *out = jret;
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
    mjson_asm_objs(ohdf, &out);

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
    mjson_asm_objs(ohdf, &out);

    cgiwrap_writef("%s(%s);\n", cb, json_object_to_json_string(out));
    TRACE_NOK(err);

    json_object_put(out);
}

void mjson_str2hdf(HDF *node, struct json_object *o)
{
    if (!node) return;
    
    char *s = hdf_obj_value(node);
    
    struct json_object *obj;
    struct array_list *list;
    enum json_type type;
    HDF *cnode;
    char tok[64];
    int i;
    
    char *key; struct json_object *val; struct lh_entry *entry;
    
    obj = o;

    if (!obj && s && *s) {
        obj = json_tokener_parse(s);
    }
    if (!obj || (int)obj < 0) return;

    type = json_object_get_type(obj);

    switch (type) {
    case json_type_boolean:
        hdf_set_int_value(node, NULL, json_object_get_boolean(obj));
        return;
    case json_type_int:
        hdf_set_int_value(node, NULL, json_object_get_int(obj));
        return;
    case json_type_double:
        sprintf(tok, "%f", json_object_get_double(obj));
        hdf_set_value(node, NULL, tok);
        return;
    case json_type_string:
        hdf_set_value(node, NULL, json_object_get_string(obj));
        return;
    case json_type_array:
        list = json_object_get_array(obj);
        for (i = 0; i < list->length; i++) {
            sprintf(tok, "%d", i);
            hdf_get_node(node, tok, &cnode);
            mjson_str2hdf(cnode, (struct json_object*)list->array[i]);
        }
        return;
    case json_type_object:
        for(entry = json_object_get_object(obj)->head;
            (entry ? (key = (char*)entry->k,
                      val = (struct json_object*)entry->v, entry) : 0);
            entry = entry->next) {
            hdf_get_node(node, key, &cnode);
            mjson_str2hdf(cnode, val);
        }
        return;
    default:
        return;
    }
}
