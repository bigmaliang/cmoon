#ifndef __MBSON_H__
#define __MBSON_H__

#include "mheads.h"

__BEGIN_DECLS

/*
 * These functions should be offer by mongo client API
 * but it doesn't right now, so, offer them!
 */
bson* mbson_new_from_jsonobj(struct json_object *obj, bool finish);
bson* mbson_new_from_string(const char *s, bool finish);
struct json_object* mbson_export_to_jsonobj(bson *doc, bool array);
char* mbson_string(bson *doc);

NEOERR* mbson_import_from_hdf(HDF *node, bson **out, bool finish);
NEOERR* mbson_export_to_hdf(HDF *node, bson *doc);

__END_DECLS
#endif    /* __MBSON_H__ */
