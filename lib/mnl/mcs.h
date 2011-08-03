#ifndef __MCS_H__
#define __MCS_H__

#include "mheads.h"

__BEGIN_DECLS

/* because of libneo_cs doesn't have cs_render_stdout */
NEOERR* mcs_outputcb(void *ctx, char *s);
/* because of libneo_cs doesn't have cs_render_to_file */
NEOERR* mcs_strcb(void *ctx, char *s);
NEOERR* mcs_str2file(STRING str, const char *file);

void mcs_rand_string(char *s, int max);
NEOERR* mcs_register_bitop_functions(CSPARSE *cs);
NEOERR* mcs_register_mkd_functions(CSPARSE *cs);
void mcs_hdf_escape_val(HDF *hdf);
void mcs_html_escape(HDF *hdf, char *name);
NEOERR* mcs_err_valid(NEOERR *err);

/*
 * build UPDATE's SET xxxx string
 * data    :IN val {aname: 'foo', pname: 'bar'}
 * node :IN key {0: 'aname', 1: 'pname'}
 * str  :OUT update colum string: aname='foo', pname='bar',
 */
NEOERR* mcs_build_upcol(HDF *data, HDF *node, STRING *str);
/*
 * str.buf is already escaped
 * just put them in query by %s, don't need $1(cause error)
 */
NEOERR* mcs_build_querycond(HDF *data, HDF *node, STRING *str, char *defstr);
NEOERR* mcs_build_incol(HDF *data, HDF *node, STRING *str);

#define MLIST_ITERATE(list, item)                                       \
    item = list->items[0];                                              \
    for (int t_rsv_i = 0; t_rsv_i < list->num; item = list->items[++t_rsv_i])

#define ITERATE_MLIST(ul)                               \
    for (int t_rsv_i = 0; t_rsv_i < ul->num; t_rsv_i++)

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
