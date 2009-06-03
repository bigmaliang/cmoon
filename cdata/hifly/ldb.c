#include "mheads.h"
#include "lheads.h"

int ldb_init(fdb_t **fdb, char *ip, char *name, unsigned int port)
{
	if (ip == NULL) ip = hdf_get_value(g_cfg, CFG_DB".ip", "127.0.0.1");
	if (name == NULL) name = hdf_get_value(g_cfg, CFG_DB".name", "test");

	if (port < 0) port = (unsigned int)hdf_get_int_value(g_cfg, CFG_DB".port", 0);

	return fdb_init_long(fdb, ip, hdf_get_value(g_cfg, CFG_DB".user", "test"),
						 hdf_get_value(g_cfg, CFG_DB".pass", "test"), name, port);
}
