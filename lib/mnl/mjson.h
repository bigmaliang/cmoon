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
void mjson_output_hdf(HDF *hdf, time_t second);
void mjson_execute_hdf(HDF *hdf, char *cb, time_t second);


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
 *
 * plan.id = 4301
 * plan.name = foo
 * plan.dad.0.x = 1
 * plan.dad.0.y = 2
 * plan.dad.1.x = 1
 * plan.dad.1.y = 2
 * plan.mun.0 = foo
 * plan.mun.1 = bar
 */
void mjson_str2hdf(HDF *node, struct json_object *o);

__END_DECLS
#endif    /* __MJSON_H__ */
