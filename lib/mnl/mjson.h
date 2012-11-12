#ifndef __MJSON_H__
#define __MJSON_H__

#include "mheads.h"

__BEGIN_DECLS

/*
 * Output {
 *     1 = fff
 *     1 {
 *         0 [type=int] = 301
 *     }
 *     foo = bar
 * }
 * will output { "1": { "0": 301 }, "foo": "bar" }
 *
 * Output [type=array] {
 *     1 = fff
 *     1 {
 *         0 [type=int] = 301
 *     }
 *     foo = bar
 * }
 * will output [ "fff", { "0": 301 }, "bar" ]
 */
NEOERR* mjson_import_from_hdf(HDF *node, struct json_object **out);

/*
 * import wraper functions
 */
void mjson_output_hdf(HDF *node, time_t second);
void mjson_execute_hdf(HDF *node, char *cb, time_t second);

/*
 *  plan = {
 *             "id": "4301",
 *             "name": "foo",
 *             "dad": [
 *                 {"x": 1, "y": 2},
 *                 {"x": 1, "y": 2},
 *              ],
 *             "mum": ["foo", "bar"]
 *  }
 * ===>
 * plan.id = 4301
 * plan.name = foo
 * plan.dad.0.x = 1
 * plan.dad.0.y = 2
 * plan.dad.1.x = 1
 * plan.dad.1.y = 2
 * plan.mun.0 = foo
 * plan.mun.1 = bar
 */
NEOERR* mjson_export_to_hdf(HDF *node, struct json_object *obj, bool drop);
/*
 * we set param str into node if str != NULL
 * or, we'll compile node's value to a json object, and set it
 */
NEOERR* mjson_string_to_hdf(HDF *node, char *str);

__END_DECLS
#endif    /* __MJSON_H__ */
