#include "mheads.h"

/*
 * Output {
 *   test {
 *     0 {
 *       t = aaa
 *     }
 *     1 {
 *       t = bbb
 *     }
 *     3 = ddd
 *   }
 *   key = 22,18,3,4999
 *   value {
 *     22 = 0
 *     18 = 5809
 *     3 = 6912
 *     4999 = not exist
 *   }
 * }
 * will output
 * { "test": [ { "t": "aaa" }, { "t": "bbb" }, { "3": "ddd" } ], "key": "22,18,3,4999", "value": { "22": "0", "18": "5809", "3": "6912", "4999": "not exist" } }
 *
 *   test {
 *     0 = aaa
 *     1 {
 *       t = bbb
 *     }
 *     3 = ddd
 *   }
 * will output
 * { "test": { "0": "aaa", "1": { "t": "bbb" }, "3": "ddd" } }
 */
void mjson_asm_objs(HDF *hdf, struct json_object *out)
{
	if (hdf == NULL)
		return;

	HDF *chi, *num;
	struct json_object *arr, *jso, *item;
	char *val, *type;
	arr = jso = NULL;

	hdf = hdf_obj_child(hdf);

	while (hdf) {
		if ((val = hdf_obj_value(hdf)) != NULL) {
			type = mutil_obj_attr(hdf, "type");
			if (type != NULL && !strcmp(type, "int")) {
				jso = json_object_new_int(atoi(val));
			} else {
				jso = json_object_new_string(val);
			}
			json_object_object_add(out, hdf_obj_name(hdf), jso);
		}

		if (hdf_obj_child(hdf) != NULL) {
			num = hdf_get_obj(hdf, "0");
			if (num != NULL) {
				/* child is array */
				arr = json_object_new_array();

				while (num !=NULL) {
					jso = json_object_new_object();

					if (hdf_obj_child(num) != NULL) {
						mjson_asm_objs(num, jso);
					} else {
						val = hdf_obj_value(num);
						if (val != NULL) {
							type = mutil_obj_attr(num, "type");
							if (type != NULL && !strcmp(type, "int")) {
								jso = json_object_new_int(atoi(val));
							} else {
								jso = json_object_new_string(val);
							}
						}
					}

					json_object_array_add(arr, jso);
					
					num = hdf_obj_next(num);
				}
				json_object_object_add(out, hdf_obj_name(hdf), arr);
			} else {
				jso = json_object_new_object();
				mjson_asm_objs(hdf, jso);
				json_object_object_add(out, hdf_obj_name(hdf), jso);
			}
		}
		
		hdf = hdf_obj_next(hdf);
	}
}

void mjson_asm_arrs(HDF *hdf, struct json_object *out)
{
	if (hdf == NULL)
		return;

	HDF *chi, *num;
	struct json_object *arr, *jso, *item;
	char *val, *type;
	arr = jso = NULL;

	hdf = hdf_obj_child(hdf);

	while (hdf) {
		if ((val = hdf_obj_value(hdf)) != NULL) {
			type = mutil_obj_attr(hdf, "type");
			if (type != NULL && !strcmp(type, "int")) {
				jso = json_object_new_int(atoi(val));
			} else {
				jso = json_object_new_string(val);
			}
			json_object_array_add(out, jso);
		}

		if (hdf_obj_child(hdf) != NULL) {
			num = hdf_get_obj(hdf, "0");
			if (num != NULL) {
				/* child is array */
				arr = json_object_new_array();

				while (num !=NULL) {
					jso = json_object_new_object();

					if (hdf_obj_child(num) != NULL) {
						mjson_asm_objs(num, jso);
					} else {
						val = hdf_obj_value(num);
						if (val != NULL) {
							type = mutil_obj_attr(num, "type");
							if (type != NULL && !strcmp(type, "int")) {
								jso = json_object_new_int(atoi(val));
							} else {
								jso = json_object_new_string(val);
							}
						}
					}

					json_object_array_add(arr, jso);
					
					num = hdf_obj_next(num);
				}
				json_object_array_add(out, arr);
			} else {
				jso = json_object_new_object();
				mjson_asm_objs(hdf, jso);
				json_object_array_add(out, jso);
			}
		}
		
		hdf = hdf_obj_next(hdf);
	}
}

void mjson_output_hdf(HDF *hdf, time_t second)
{
	if (second > 0) {
		mmisc_cache_headers(second);
	}
	
	NEOERR *err = cgiwrap_writef("Content-Type: text/html; charset=UTF-8\r\n\r\n");
	if (err != STATUS_OK) nerr_ignore(&err);
	HDF *ohdf = hdf_get_obj(hdf, PRE_OUTPUT);
	if (ohdf == NULL) {
		return;
	}

	struct json_object *out;
	if (hdf_get_obj(ohdf, "0")) {
		out = json_object_new_array();
		mjson_asm_arrs(ohdf, out);
	} else {
		out = json_object_new_object();
		mjson_asm_objs(ohdf, out);
	}

	err = cgiwrap_writef("%s\n", json_object_to_json_string(out));
	if (err != STATUS_OK) nerr_ignore(&err);
	json_object_put(out);
}
void mjson_execute_hdf(HDF *hdf, char *cb, time_t second)
{
	if (second > 0) {
		mmisc_cache_headers(second);
	}
	
	NEOERR *err = cgiwrap_writef("Content-Type: text/html; charset=UTF-8\r\n\r\n");
	if (err != STATUS_OK) nerr_ignore(&err);
	HDF *ohdf = hdf_get_obj(hdf, PRE_OUTPUT);
	if (ohdf == NULL) {
		return;
	}

	struct json_object *out;
	if (hdf_get_obj(ohdf, "0")) {
		out = json_object_new_array();
		mjson_asm_arrs(ohdf, out);
	} else {
		out = json_object_new_object();
		mjson_asm_objs(ohdf, out);
	}

	cgiwrap_writef("%s(%s);\n", cb, json_object_to_json_string(out));
	if (err != STATUS_OK) nerr_ignore(&err);
	json_object_put(out);
}
