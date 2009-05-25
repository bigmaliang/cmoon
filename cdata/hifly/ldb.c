#include "mheads.h"
#include "lheads.h"

int ldb_init(fdb_t **fdb, char *ip, char *name)
{
	return fdb_init_long(fdb, ip, DB_USER, DB_PASS, name);
}

