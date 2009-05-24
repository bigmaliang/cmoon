#include "mheads.h"

void mjson_asm_objs(HDF *hdf, struct json_object *out)
{
	if (hdf == NULL)
		return;

	HDF *chi, *num;
	struct json_object *arr, *jso;
	arr = jso = NULL;
	/* process hdf.[0-9].* */
	num = hdf_get_obj(hdf, "0");
	if (num != NULL) {
		arr = json_object_new_array();
	}
	while (num != NULL && hdf_obj_child(num) != NULL) {
		struct json_object *item = json_object_new_object();
		mjson_asm_objs(hdf_obj_child(num), item);
		json_object_array_add(arr, item);
		num = hdf_obj_next(num);
	}
	if (arr != NULL) {
		json_object_object_add(out, hdf_obj_name(hdf), arr);
		goto next_obj;
	}

	/* process hdf.[^0-9].* */
	chi = hdf_obj_child(hdf);
	if (chi != NULL) {
		mjson_asm_objs(chi, out);
	}

	/* process leaf hdf */
	char *val = hdf_obj_value(hdf);
	if (val != NULL) {
		jso = json_object_new_string(val);
		json_object_object_add(out, hdf_obj_name(hdf), jso);
	}

next_obj:
	mjson_asm_objs(hdf_obj_next(hdf), out);
}

void mjson_output_hdf(HDF *hdf)
{
	//printf("Content-Type: application/jsonrequest; charset=UTF-8\r\n\r\n");
	printf("Content-Type: text/html; charset=UTF-8\r\n\r\n");
	HDF *ohdf = hdf_get_child(hdf, PRE_OUTPUT);
	if (ohdf == NULL) {
		mtc_warn("output empty json data");
		return;
	}

	struct json_object *out = json_object_new_object();
	if (out == NULL)
		return;


	mjson_asm_objs(ohdf, out);
	printf("%s\n", json_object_to_json_string(out));
	json_object_put(out);
}
