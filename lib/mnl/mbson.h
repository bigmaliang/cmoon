#ifndef __MBSON_H__
#define __MBSON_H__

#include "mheads.h"

__BEGIN_DECLS

/*
 * These functions should be offer by mongo client API
 * but it doesn't right now, so, offer them!
 */
bson* mbson_new_from_jsonobj(struct json_object *obj, bool finish, bool drop);
bson* mbson_new_from_string(const char *s, bool finish);
struct json_object* mbson_export_to_jsonobj(bson *doc, bool array);
char* mbson_string(bson *doc);

NEOERR* mbson_import_from_hdf(HDF *node, bson **out, bool finish);
enum {
    MBSON_EXPORT_NONE = 0,
    /* input bson doc isn't an object, an array */
    MBSON_EXPORT_ARRAY = 1 << 1,
    /* set node's [type='xxx'] information, for later convert useage */
    MBSON_EXPORT_TYPE = 1 << 2
};
NEOERR* mbson_export_to_hdf(HDF *node, bson *doc, char *key, int flag, bool drop);
NEOERR* mbson_export_to_int_hdf(HDF *node, bson *doc, int key, int flag, bool drop);

__END_DECLS
#endif    /* __MBSON_H__ */
