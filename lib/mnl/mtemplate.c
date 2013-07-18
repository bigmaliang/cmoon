#include "mheads.h"

static HDF *m_loadpath = NULL;
static char *m_tplpath = NULL;

#ifdef __MACH__
static int mtpl_config(struct dirent *ent)
#else
static int mtpl_config(const struct dirent *ent)
#endif
{
    if (reg_search(".*.hdf", ent->d_name))
        return 1;
    else
        return 0;
}

static NEOERR* mtpl_set_loadpath(CSPARSE *cs)
{
    HDF *node;

    MCS_NOT_NULLA(cs);

    if (!m_loadpath) return STATUS_OK;
    
    node = hdf_obj_child(m_loadpath);
    while (node) {
        hdf_set_valuef(cs->hdf, "hdf.loadpath.%s=%s",
                       hdf_obj_name(node), hdf_obj_value(node));

        node = hdf_obj_next(node);
    }

    return STATUS_OK;
}

NEOERR* mtpl_set_tplpath(char *dir)
{
    m_tplpath = dir;

    return nerr_pass(mtpl_append_loadpath(dir));
}

NEOERR* mtpl_append_loadpath(char *dir)
{
    HDF *node;
    int cnt;
    NEOERR *err;

    MCS_NOT_NULLA(dir);
    
    if (!m_loadpath) {
        err = hdf_init(&m_loadpath);
        if (err != STATUS_OK) return nerr_pass(err);
    }

    cnt = 0;
    node = hdf_obj_child(m_loadpath);
    while (node) {
        cnt++;
        node = hdf_obj_next(node);
    }

    hdf_set_valuef(m_loadpath, "%d=%s", cnt, dir);

    return STATUS_OK;
}

static NEOERR* mtpl_InConfigRend_parse_file(char *dir, char *name,
                                           HDF *value_node, HDF *layout_node,
                                           HASH *layout_hash)
{
    HDF *node, *child, *obj;
    char fname[_POSIX_PATH_MAX], *tpl;
    CSPARSE *cs;
    NEOERR *err;

    err = hdf_init(&node);
    if (err != STATUS_OK) return nerr_pass(err);

    snprintf(fname, sizeof(fname), "%s/%s", dir, name);
    err = hdf_read_file(node, fname);
    if (err != STATUS_OK) return nerr_pass(err);

    child = hdf_obj_child(node);
    while (child) {
        mtc_dbg("parse node %s", hdf_obj_name(child));

        /*
         * value node
         */
        obj = hdf_get_obj(child, PRE_VALUESET);
        if (obj) {
            hdf_copy(value_node, hdf_obj_name(child), obj);
        }

        /*
         * layout node
         */
        obj = hdf_get_obj(child, PRE_LAYOUT);
        if (obj) {
            hdf_copy(layout_node, hdf_obj_name(child), obj);
        }

        /*
         * layout hash
         */
        obj = hdf_get_child(child, PRE_LAYOUT);
        while (obj) {
            tpl = hdf_obj_value(obj);
            snprintf(fname, sizeof(fname), "%s/%s", m_tplpath, tpl);

            err = cs_init(&cs, hdf_get_obj(child, PRE_DATASET));
            JUMP_NOK(err, wnext);

            err = mtpl_set_loadpath(cs);
            JUMP_NOK(err, wnext);

            err = cgi_register_strfuncs(cs);
            JUMP_NOK(err, wnext);
            err = mcs_register_bitop_functions(cs);
            JUMP_NOK(err, wnext);
            err = mcs_register_mkd_functions(cs);
            JUMP_NOK(err, wnext);
            err = mcs_register_string_uslice(cs);
            JUMP_NOK(err, wnext);

            err = cs_parse_file(cs, fname);
            JUMP_NOK(err, wnext);

            err = hash_insertf(layout_hash, (void*)cs, "%s_%s",
                               hdf_obj_name(child), hdf_obj_name(obj));
            JUMP_NOK(err, wnext);

        wnext:
            obj = hdf_obj_next(obj);
        }

        child = hdf_obj_next(child);
    }
    
    return STATUS_OK;
}

static NEOERR* mtpl_InConfigRend_parse_dir(char *dir,
                                          HDF *valuen, HDF *layoutn, HASH *layouth)
{
    struct dirent **eps = NULL;
    int n;
    NEOERR *err;

    MCS_NOT_NULLA(dir);
    MCS_NOT_NULLC(valuen, layoutn, layouth);
    
    n = scandir(dir, &eps, mtpl_config, alphasort);
    for (int i = 0; i < n; i++) {
        mtc_dbg("parse file %s", eps[i]->d_name);
        err = mtpl_InConfigRend_parse_file(dir, eps[i]->d_name,
                                          valuen, layoutn, layouth);
        TRACE_NOK(err);
        free(eps[i]);
    }

    if (n > 0) free(eps);
    else mtc_warn("no .hdf file found in %s", dir);

    return STATUS_OK;
}

NEOERR* mtpl_InConfigRend_init(char *dir, char *key, HASH **datah)
{
    HDF *value_node, *layout_node;
    HASH *layout_hash;
    NEOERR *err;

    MCS_NOT_NULLC(dir, key, datah);

    if (*datah == NULL) {
        err = hash_init(datah, hash_str_hash, hash_str_comp, NULL);
        if (err != STATUS_OK) return nerr_pass(err);
    }

    value_node = hash_lookupf(*datah, "icr_%s_value_node", key);
    if (value_node) {
        mtc_warn("%s already inited", key);
        return STATUS_OK;
    }

    err = hdf_init(&value_node);
    if (err != STATUS_OK) return nerr_pass(err);
    err = hash_insertf(*datah, (void*)value_node, "icr_%s_value_node", key);
    if (err != STATUS_OK) return nerr_pass(err);
    
    err = hdf_init(&layout_node);
    if (err != STATUS_OK) return nerr_pass(err);
    err = hash_insertf(*datah, (void*)layout_node, "icr_%s_layout_node", key);
    if (err != STATUS_OK) return nerr_pass(err);
    
    err = hash_init(&layout_hash, hash_str_hash, hash_str_comp, NULL);
    if (err != STATUS_OK) return nerr_pass(err);
    err = hash_insertf(*datah, (void*)layout_hash, "icr_%s_layout_hash", key);
    if (err != STATUS_OK) return nerr_pass(err);

    err = mtpl_InConfigRend_parse_dir(dir, value_node, layout_node, layout_hash);
    if (err != STATUS_OK) return nerr_pass(err);

    return STATUS_OK;
}

NEOERR* mtpl_InConfigRend_get(HDF *out, HDF *in, char *key, char *name, HASH *datah)
{
    STRING str; string_init(&str);
    HDF *value_node, *layout_node;
    HDF *node, *obj;
    HASH *layout_hash;
    CSPARSE *cs;
    NEOERR *err;

    MCS_NOT_NULLC(out, key, datah);

    value_node = hash_lookupf(datah, "icr_%s_value_node", key);
    layout_node = hash_lookupf(datah, "icr_%s_layout_node", key);
    layout_hash = hash_lookupf(datah, "icr_%s_layout_hash", key);
    
    MCS_NOT_NULLC(value_node, layout_node, layout_hash);

    /*
     * copy in's valueset
     */
    node = hdf_get_obj(in, PRE_VALUESET);
    if (node) {
        err = hdf_copy(out, NULL, node);
    }

    /*
     * copy & replace config hdf's valueset
     */
    node = hdf_get_obj(value_node, name);
    if (node) {
        err = mcs_hdf_copy_rep(out, NULL, node, hdf_get_obj(in, PRE_VALUEREP));
        if (err != STATUS_OK) return nerr_pass(err);
    }

    /*
     * rend & copy config hdf's layout
     */
    node = hdf_get_child(layout_node, name);
    while (node) {
        cs = hash_lookupf(layout_hash, "%s_%s", name, hdf_obj_name(node));
        if (!cs) return nerr_raise(NERR_ASSERT, "Layout %s_%s not found under %s",
                                   name, hdf_obj_name(node), key);

        /* cs->hdf is the config file's dataset on cs_init() */
        obj = cs->hdf;
        if (obj) hdf_copy(in, PRE_DATASET, obj);
        cs->hdf = hdf_get_obj(in, PRE_DATASET);
        err = cs_render(cs, &str, mcs_strcb);
        if (err != STATUS_OK) return nerr_pass(err);

        hdf_set_value(out, hdf_obj_name(node), str.buf);

        /* restore cs->hdf */
        cs->hdf = obj;
        string_clear(&str);
        
        node = hdf_obj_next(node);
    }

    return STATUS_OK;
}

void mtpl_InConfigRend_destroy(HASH *datah)
{
    char *key = NULL, *buf;

    if (!datah) return;
    
    buf = (char*)hash_next(datah, (void**)&key);
    
    while (buf != NULL) {
        /* TODO free them */
        //free(buf);
        buf = hash_next(datah, (void**)&key);
    }

    hash_destroy(&datah);
}
