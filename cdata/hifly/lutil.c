#include "mheads.h"
#include "lheads.h"

bool lutil_is_userdata_key(char *key)
{
	char *p = key+strlen(key)-1;
	while (*p != '_' && p != key) {
		if (!isdigit(*p))
			return false;
		p--;
	}
	return true;
}

bool lutil_user_has_power(const char *user, const char *pass)
{
	if (user == NULL || pass == NULL) return false;

	HDF *node = hdf_get_obj(g_cfg, CFG_SET_CALLER);
	if (node == NULL) {
		mtc_err("%s config not found", CFG_SET_CALLER);
		return false;
	}

	node = hdf_obj_child(node);
	while (node != NULL) {
		if (!strcmp(user, hdf_get_value(node, "user", "impossible")) &&
			!strcmp(pass, hdf_get_value(node, "pass", "impossible"))) {
			return true;
		}

		node = hdf_obj_next(node);
	}
	return false;
}
