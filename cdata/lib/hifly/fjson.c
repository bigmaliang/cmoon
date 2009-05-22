#include "fheads.h"

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
void fjson_asm_objs(HDF *hdf, struct json_object *out)
{
	if (hdf == NULL)
		return;

	HDF *chi, *num;
	struct json_object *arr, *jso, *item;
	char *val;
	arr = jso = NULL;

	/* process child node */
	chi = hdf_obj_child(hdf);
	if (chi != NULL) {
		num = hdf_get_obj(hdf, "0");
		if (num != NULL) {
			if (hdf_obj_child(num) != NULL) {
				/* child is array */
				arr = json_object_new_array();
				while (num != NULL ) {
					item = json_object_new_object();
					if (hdf_obj_child(num) != NULL) {
						fjson_asm_objs(hdf_obj_child(num), item);
					} else {
						val = hdf_obj_value(num);
						if (val != NULL) {
							char *type = futil_obj_attr(num, "type");
							if (type != NULL && !strcmp(type, "int")) {
								jso = json_object_new_int(atoi(val));
							} else {
								jso = json_object_new_string(val);
							}
							json_object_object_add(item, hdf_obj_name(num), jso);
						}
					}
					json_object_array_add(arr, item);
					num = hdf_obj_next(num);
				}
				json_object_object_add(out, hdf_obj_name(hdf), arr);
				goto next_obj;
			}
		}
		jso = json_object_new_object();
		fjson_asm_objs(chi, jso);
		json_object_object_add(out, hdf_obj_name(hdf), jso);
	}

	/* process leaf hdf */
	val = hdf_obj_value(hdf);
	if (val != NULL) {
		char *type = futil_obj_attr(hdf, "type");
		if (type != NULL && !strcmp(type, "int")) {
			jso = json_object_new_int(atoi(val));
		} else {
			jso = json_object_new_string(val);
		}
		json_object_object_add(out, hdf_obj_name(hdf), jso);
	}

next_obj:
	fjson_asm_objs(hdf_obj_next(hdf), out);
}

void fjson_output_hdf(HDF *hdf)
{
	NEOERR *err = cgiwrap_writef("Content-Type: text/html; charset=UTF-8\r\n\r\n");
	if (err != STATUS_OK) nerr_ignore(&err);
	HDF *ohdf = hdf_get_child(hdf, PRE_OUTPUT);
	if (ohdf == NULL) {
		return;
	}

	struct json_object *out = json_object_new_object();
	if (out == NULL)
		return;

	fjson_asm_objs(ohdf, out);
	err = cgiwrap_writef("%s\n", json_object_to_json_string(out));
	if (err != STATUS_OK) nerr_ignore(&err);
	json_object_put(out);
}
void fjson_execute_hdf(HDF *hdf, char *cb)
{
	NEOERR *err = cgiwrap_writef("Content-Type: text/html; charset=UTF-8\r\n\r\n");
	if (err != STATUS_OK) nerr_ignore(&err);
	HDF *ohdf = hdf_get_child(hdf, PRE_OUTPUT);
	if (ohdf == NULL) {
		return;
	}

	struct json_object *out = json_object_new_object();
	if (out == NULL)
		return;

	fjson_asm_objs(ohdf, out);
	cgiwrap_writef("%s(%s);\n", cb, json_object_to_json_string(out));
	if (err != STATUS_OK) nerr_ignore(&err);
	json_object_put(out);
}
