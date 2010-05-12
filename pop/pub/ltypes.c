#include "mheads.h"
#include "lheads.h"

#define SAFE_FREE(str)							\
	do {										\
		if (str != NULL)						\
			free(str);							\
	} while (0)

int session_init(HDF *hdf, HASH *dbh, session_t **ses)
{
	session_t *lses;
	
	*ses = NULL;

	lses = calloc(1, sizeof(session_t));
	if (lses == NULL) {
		mtc_err("calloc memory for session_t failure");
		return RET_RBTOP_MEMALLOCE;
	}

	*ses = lses;

	hdf_get_copy(hdf, PRE_COOKIE".uname", &lses->uname, NULL);
	hdf_set_copy(hdf, PRE_REQ_URI_RW_HDF, PRE_REQ_URI_RW);
	mmisc_str_repchr(hdf_get_value(hdf, PRE_REQ_URI_RW_HDF, "NULL"), '/', '.');
	
    /* process cache */
	HDF *node = hdf_get_obj(g_cfg, PRE_CFG_FILECACHE".0");
	while (node != NULL) {
		if (reg_search(hdf_get_value(node, "uri", "NULL"),
					   hdf_get_value(hdf, PRE_REQ_URI_RW, "404"))) {
			lses->tm_cache_browser = hdf_get_int_value(node, "tm_cache", 0);
			break;
		}
		node = hdf_obj_next(node);
	}

	return RET_RBTOP_OK;
}

void session_destroy(session_t **ses)
{
	session_t *lses;
	if (ses == NULL) return;
	lses = *ses;

	if (lses == NULL) return;

    SAFE_FREE(lses->uname);

	free(lses);
	lses = NULL;
}
