#ifndef __MCS_H__
#define __MCS_H__

#include "mheads.h"

__BEGIN_DECLS

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

unsigned int mcs_get_uint_value(HDF *hdf, const char *name, unsigned int defval);
float mcs_get_float_value(HDF *hdf, const char *name, float defval);
NEOERR* mcs_set_uint_value(HDF *hdf, const char *name, unsigned int value);
NEOERR* mcs_set_float_value(HDF *hdf, const char *name, float value);

HDF*    mcs_hdf_getf(HDF *node, char *fmt, ...)
                     ATTRIBUTE_PRINTF(2, 3);
NEOERR* mcs_hdf_copyf(HDF *dst, HDF *src, char *fmt, ...)
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

char* mcs_hdf_attr(HDF *hdf, char *name, char*key);
char* mcs_obj_attr(HDF *hdf, char*key);

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

__END_DECLS
#endif    /* __MCS_H__ */
