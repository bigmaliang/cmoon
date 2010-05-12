#include "mheads.h"
#include "lheads.h"

int levt_init(HASH **evth)
{
	mevent_t *evt;
	char *ename;
	HDF *node;
	HASH *levth;
	NEOERR *err;

	node = hdf_get_obj(g_cfg, "Mevent");
	if (node == NULL) {
		mtc_err("Mevent config not found");
		return RET_RBTOP_INITE;
	}
	
	err = hash_init(&levth, hash_str_hash, hash_str_comp);
	RETURN_V_NOK(err, RET_RBTOP_INITE);

	node = hdf_obj_child(node);
	bool filled = false;
	while (node != NULL) {
		ename = hdf_obj_value(node);
		evt = mevent_init_plugin(ename, REQ_CMD_APPINFO, FLAGS_SYNC);
		if (evt) {
			hash_insert(levth, (void*)strdup(ename), (void*)evt);
			filled = true;
		}
		
		node = hdf_obj_next(node);
	}

	if (!filled) {
		mtc_err("no valid event backend");
		return RET_RBTOP_INITE;
	}
	*evth = levth;
	return RET_RBTOP_OK;
}

void levt_destroy(HASH *evth)
{
	char *key = NULL;
	
	mevent_t *evt = (mevent_t*)hash_next(evth, (void**)&key);

	while (evt != NULL) {
		mevent_free(evt);
		evt = hash_next(evth, (void**)&key);
	}

	hash_destroy(&evth);
}

