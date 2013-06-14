#ifndef __MCS_H__
#define __MCS_H__

#include "mheads.h"

__BEGIN_DECLS

typedef enum {
    CNODE_TYPE_STRING = 100,
    CNODE_TYPE_BOOL,
    CNODE_TYPE_INT,
    CNODE_TYPE_INT64,
    CNODE_TYPE_FLOAT,
    CNODE_TYPE_DATETIME, /**< 8byte timestamp; milliseconds since Unix epoch */
    CNODE_TYPE_TIMESTAMP, /**< 4bytes increment + 4bytes timestamp */
    CNODE_TYPE_OBJECT,
    CNODE_TYPE_ARRAY,
    CNODE_TYPE_JS,
    CNODE_TYPE_SYMBOL,
    CNODE_TYPE_OID, /**< 12byte ObjectID (uint) */
    
    CNODE_TYPE_POINT = 120,
    CNODE_TYPE_BOX,
    CNODE_TYPE_PATH,
    CNODE_TYPE_TIME,
    CNODE_TYPE_BITOP
} CnodeType;

#define MLIST_ITERATE(list, item)                                       \
    item = list->items[0];                                              \
    for (int t_rsv_i = 0; t_rsv_i < list->num; item = list->items[++t_rsv_i])

#define ITERATE_MLIST(ul)                               \
    for (int t_rsv_i = 0; t_rsv_i < ul->num; t_rsv_i++)

#define MCS_NOT_NULLA(pa)                                       \
    if (!pa) return nerr_raise(NERR_ASSERT, "paramter null");
#define MCS_NOT_NULLB(pa, pb)                                           \
    if (!pa || !pb) return nerr_raise(NERR_ASSERT, "paramter null");
#define MCS_NOT_NULLC(pa, pb, pc)                                       \
    if (!pa || !pb || !pc) return nerr_raise(NERR_ASSERT, "paramter null");

void* hash_lookupf(HASH *table, char *fmt, ...)
                   ATTRIBUTE_PRINTF(2, 3);
NEOERR* hash_insertf(HASH *table, void *data, char *fmt, ...)
                     ATTRIBUTE_PRINTF(3, 4);

/*
 * because of libneo_cs doesn't have
 * cs_render_stdout
 * cs_render_to_file
 */
NEOERR* mcs_outputcb(void *ctx, char *s);
NEOERR* mcs_strcb(void *ctx, char *s);
NEOERR* mcs_str2file(STRING str, const char *file);

NEOERR* mcs_register_bitop_functions(CSPARSE *cs);
NEOERR* mcs_register_mkd_functions(CSPARSE *cs);
NEOERR* mcs_register_string_uslice(CSPARSE *cs);
NEOERR* mcs_register_upload_parse_cb(CGI *cgi, void *rock);

int  mcs_get_child_num(HDF *hdf, char *name);
/*
 * n = 0 for 1st child
 */
HDF* mcs_get_nth_child(HDF *hdf, char *name, int n);
HDF* mcs_obj_nth_child(HDF *hdf, int n);
HDF* mcs_get_objf(HDF *hdf, char *fmt, ...)
                  ATTRIBUTE_PRINTF(2, 3);
NEOERR* mcs_get_nodef(HDF *hdf, HDF **rnode, char *fmt, ...)
                      ATTRIBUTE_PRINTF(3, 4);
NEOERR* mcs_copyf(HDF *dst, HDF *src, char *fmt, ...)
                      ATTRIBUTE_PRINTF(3, 4);
NEOERR* mcs_remove_treef(HDF *hdf, char *fmt, ...)
                         ATTRIBUTE_PRINTF(2, 3);
int mcs_get_child_numf(HDF *hdf, char *fmt, ...)
                       ATTRIBUTE_PRINTF(2, 3);
HDF* mcs_get_nth_childf(HDF *hdf, int n, char *fmt, ...)
                        ATTRIBUTE_PRINTF(3, 4);


unsigned int mcs_get_uint_value(HDF *hdf, char *name, unsigned int defval);
float mcs_get_float_value(HDF *hdf, char *name, float defval);
int mcs_get_int_valuef(HDF *hdf, int defval, char *fmt, ...)
                       ATTRIBUTE_PRINTF(3, 4);
int64_t mcs_get_int64_value(HDF *hdf, char *name, int64_t defval);
NEOERR* mcs_set_int64_value(HDF *hdf, char *name, int64_t val);
NEOERR* mcs_set_uint_value(HDF *hdf, char *name, unsigned int value);
NEOERR* mcs_set_float_value(HDF *hdf, char *name, float value);

NEOERR* mcs_set_value_with_type(HDF *hdf, char *name, char *value, CnodeType type);
NEOERR* mcs_set_int_value_with_type(HDF *hdf, char *name, int value, CnodeType type);
NEOERR* mcs_set_int64_value_with_type(HDF *hdf, char *name, int64_t value, CnodeType type);
NEOERR* mcs_set_float_value_with_type(HDF *hdf, char *name, float value, CnodeType type);

int mcs_add_int_value(HDF *node, char *key, int val);
int mcs_add_int_valuef(HDF *node, int val, char *fmt, ...)
                       ATTRIBUTE_PRINTF(3, 4);
char* mcs_append_string_value(HDF *node, char *key, char *str);
char* mcs_append_string_valuef(HDF *node, char *key, char *sfmt, ...)
                               ATTRIBUTE_PRINTF(3, 4);
char* mcs_prepend_string_value(HDF *node, char *key, char *str);
char* mcs_prepend_string_valuef(HDF *node, char *key, char *sfmt, ...)
                                ATTRIBUTE_PRINTF(3, 4);
/*
 * in:
 *
 * data {
 *    NeedReplaceA = foo
 *    NeedReplaceB = bar
 * }
 * dst {
 *    class = senior
 *    comment {
 *        nick = you have a NeedReplaceA comment
 *    }
 *    remark = when you are in NeedReplaceB...
 * }
 *
 * out:
 *
 * dst {
 *    class = senior
 *    comment {
 *        nick = you have a foo comment
 *    }
 *    remark = when you are in bar...
 * }
 */
void    mcs_hdf_rep(HDF *data, HDF *dst);
/* copy src hdf to dst.name hdf, and replace dst.name use data hdf */
NEOERR* mcs_hdf_copy_rep(HDF *dst, char *name, HDF *src, HDF *data);

/*
 * in:
 *
 * src = $level, $level.name,  name.$level.in$desc,  or name.$level
 * data {
 *     level = 100
 *     name = test data
 *     desc = a desc
 * }
 * c = $
 *
 * out
 *
 * 100, 100.name, name.100.ina desc, or name.100
 */
char* mcs_repstr_byhdf(char *src, char c, HDF *data);

char* mcs_hdf_attr(HDF *hdf, char *name, char*key);
char* mcs_obj_attr(HDF *hdf, char*key);
NEOERR* mcs_set_int_attr(HDF *hdf, char *name, char *key, int val);
NEOERR* mcs_set_int_attrr(HDF *hdf, char *name, char *key, int val);
int mcs_get_int_attr(HDF *hdf, char *name, char *key, int defval);

NEOERR* mcs_err_valid(NEOERR *err);

#define DIE_NOK_CGI(err)                        \
    if (err != STATUS_OK) {                     \
        STRING zstra;    string_init(&zstra);   \
        nerr_error_traceback(err, &zstra);      \
        mtc_err("%s", zstra.buf);               \
        string_clear(&zstra);                   \
        cgi_neo_error(cgi, err);                \
        nerr_ignore(&err);                      \
        exit(-1);                               \
    }

#define JUMP_NOK_CGI(err, label)                \
    if (err != STATUS_OK) {                     \
        STRING zstra;    string_init(&zstra);   \
        nerr_error_traceback(err, &zstra);      \
        mtc_err("%s", zstra.buf);               \
        string_clear(&zstra);                   \
        cgi_neo_error(cgi, err);                \
        nerr_ignore(&err);                      \
        goto label;                             \
    }

#define JUMP_NOK(err, label)                    \
    if (err != STATUS_OK) {                     \
        STRING zstra;    string_init(&zstra);   \
        nerr_error_traceback(err, &zstra);      \
        mtc_err("%s", zstra.buf);               \
        string_clear(&zstra);                   \
        nerr_ignore(&err);                      \
        goto label;                             \
    }

#define DIE_NOK_MTL(err)                        \
    if (err != STATUS_OK) {                     \
        STRING zstra;    string_init(&zstra);   \
        nerr_error_traceback(err, &zstra);      \
        mtc_err("%s", zstra.buf);               \
        string_clear(&zstra);                   \
        nerr_ignore(&err);                      \
        exit(-1);                               \
    }

#define CONTINUE_NOK(err)                       \
    if (err != STATUS_OK) {                     \
        STRING zstra;    string_init(&zstra);   \
        nerr_error_traceback(err, &zstra);      \
        mtc_err("%s", zstra.buf);               \
        string_clear(&zstra);                   \
        nerr_ignore(&err);                      \
        continue;                               \
    }

#define RETURN_NOK(err)                         \
    if (err != STATUS_OK) {                     \
        STRING zstra;    string_init(&zstra);   \
        nerr_error_traceback(err, &zstra);      \
        mtc_err("%s", zstra.buf);               \
        string_clear(&zstra);                   \
        nerr_ignore(&err);                      \
        return;                                 \
    }

#define RETURN_V_NOK(err, v)                    \
    if (err != STATUS_OK) {                     \
        STRING zstra;    string_init(&zstra);   \
        nerr_error_traceback(err, &zstra);      \
        mtc_err("%s", zstra.buf);               \
        string_clear(&zstra);                   \
        nerr_ignore(&err);                      \
        return v;                               \
    }

#define TRACE_NOK(err)                          \
    if (err != STATUS_OK) {                     \
        STRING zstra;    string_init(&zstra);   \
        nerr_error_traceback(err, &zstra);      \
        mtc_err("%s", zstra.buf);               \
        string_clear(&zstra);                   \
        nerr_ignore(&err);                      \
    }

#define OUTPUT_NOK(err)                         \
    if (err != STATUS_OK) {                     \
        STRING zstra;    string_init(&zstra);   \
        nerr_error_traceback(err, &zstra);      \
        printf("%s", zstra.buf);                \
        string_clear(&zstra);                   \
        nerr_ignore(&err);                      \
    }

#define TRACE_HDF(node)                         \
    do {                                        \
        STRING zstra;    string_init(&zstra);   \
        hdf_dump_str(node, NULL, 2, &zstra);    \
        mtc_foo("%s", zstra.buf);               \
        string_clear(&zstra);                   \
    } while (0)

/*
 * a set of macros for performance purpose. see mjson_export_to_hdf()
 */
#define MCS_SET_INT_ATTR(hdf, name, key, val)   \
    do {                                        \
        char ztoka[64];                         \
        snprintf(ztoka, 64, "%d", val);         \
        hdf_set_attr(hdf, name, key, ztoka);    \
    } while (0)
#define MCS_SET_INT_ATTRR(hdf, name, key, val)  \
    do {                                        \
        char ztoka[64];                         \
        snprintf(ztoka, 64, "%d", val);         \
        if (!hdf_get_value(hdf, name, NULL))    \
            hdf_set_value(hdf, name, "foo");    \
        hdf_set_attr(hdf, name, key, ztoka);    \
    } while (0)
#define MCS_GET_INT_ATTR(hdf, name, key, defval, v) \
    do {                                            \
        char *zsa = mcs_hdf_attr(hdf, name, key);   \
        if (zsa) v = atoi(zsa);                     \
        else v = defval;                            \
    } while (0)

__END_DECLS
#endif    /* __MCS_H__ */
