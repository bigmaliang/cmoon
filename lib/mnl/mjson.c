#include "mheads.h"

/*
 * Output {
 *     1 = fff
 *     1 {
 *         0 [type=int] = 301
 *     }
 *     foo = bar
 * }
 * will output { "1": { "0": 301 }, "foo": "bar" }
 *
 * Output [type=array] {
 *     1 = fff
 *     1 {
 *         0 [type=int] = 301
 *     }
 *     foo = bar
 * }
 * will output [ "fff", { "0": 301 }, "bar" ]
 */
void mjson_asm_objs(HDF *hdf, struct json_object **out)
{
    if (hdf == NULL)
        return;

    struct json_object *jret, *jso;
    char *val, *type;
    bool array = false;

    type = mutil_obj_attr(hdf, "type");
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
            type = mutil_obj_attr(hdf, "type");
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
        mmisc_cache_headers(second);
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
        mmisc_cache_headers(second);
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
