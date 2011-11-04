#include "mheads.h"

/* global file name for trace info write to */
static char g_fn[LEN_FN] = "";
static FILE *g_fp = NULL;
static char linebuf[2096];
static char *g_trace_level[TC_LEVELS] = {"DIE", "MESSAGE", "ERROR", "WARNING", "INFO", "DEBUG", "NOISE"};

static void trace_shift_file()
{
    struct stat fs;
    if (stat(g_fn, &fs) == -1)
        return;
    if (fs.st_size < TC_MAX_SIZE)
        return;

    int i;
    char ofn[LEN_FN], nfn[LEN_FN];

    if (g_fp != NULL)
        fclose(g_fp);

    for (i = TC_MAX_NUM-1; i > 1; i--) {
        sprintf(ofn, "%s.%d", g_fn, i-1);
        sprintf(nfn, "%s.%d", g_fn, i);
        rename(ofn, nfn);
    }
    if (TC_MAX_NUM > 1) {
        strcpy(ofn, g_fn);
        sprintf(nfn, "%s.1", g_fn);
        rename(ofn, nfn);
    }

    g_fp = fopen(g_fn, "a+");
}

void mtc_init(const char *fn)
{
    strncpy(g_fn, fn, sizeof(g_fn)-4);
    strcat(g_fn, ".log");
    if (g_fp != NULL)
        fclose(g_fp);
    g_fp = fopen(g_fn, "a+");
    if (g_fp != NULL) setvbuf(g_fp, linebuf, _IOLBF, 2096);
    atexit(mtc_leave);
}
void mtc_leave()
{
    if (g_fp != NULL)
        fclose(g_fp);
    g_fp = NULL;
    memset(g_fn, 0x0, sizeof(g_fn));
}

bool mtc_msg(const char *func, const char *file, long line,
             int level, const char *format, ...)
{
    int dftlv = hdf_get_int_value(g_cfg, PRE_CONFIG".trace_level", TC_DEFAULT_LEVEL);
    if (level > dftlv)
        return true;
    
    if (g_fp == NULL)
        return false;

    va_list ap;
    char tm[LEN_TM];
    if (!mmisc_getdatetime(tm, sizeof(tm), "%F %T", 0))
        return false;

    fprintf(g_fp, "[%s]", tm);
    fprintf(g_fp, "[%s]", g_trace_level[level]);
    fprintf(g_fp, "[%s:%li %s] ", file, line, func);

    va_start(ap, (void*)format);
    vfprintf(g_fp, format, ap);
    va_end(ap);

    fprintf(g_fp, "\n");

    trace_shift_file();
    return true;
}

NEOERR* mcs_build_upcol(HDF *data, HDF *node, STRING *str)
{
    if (!data || !node || !str) return nerr_raise(NERR_ASSERT, "param err");
    
    char *name, *col, *val, *esc, *require, *clen, *type;

    node = hdf_obj_child(node);
    
    while (node) {
        name = hdf_obj_name(node);
        col = hdf_obj_value(node);
        val = hdf_get_value(data, name, NULL);
        require = mutil_obj_attr(node, "require");
        clen = mutil_obj_attr(node, "maxlen");
        type = mutil_obj_attr(node, "type");
        if (val && *val) {
            if (type == NULL || !strcmp(type, "str")) {
                mutil_real_escape_string_nalloc(&esc, val, strlen(val));
                if (str->len <= 0) {
                    if (clen)
                        string_appendf(str, " %s='%s'::varchar(%d) ",
                                       col, esc, atoi(clen));
                    else
                        string_appendf(str, " %s='%s' ", col, esc);
                    free(esc);
                } else {
                    if (clen)
                        string_appendf(str, " , %s='%s'::varchar(%d) ",
                                       col, esc, atoi(clen));
                    else
                        string_appendf(str, " , %s='%s' ", col, esc);
                    free(esc);
                }
            } else if (!strcmp(type, "int")) {
                if (str->len <= 0)
                    string_appendf(str, " %s=%d ", col, atoi(val));
                else
                    string_appendf(str, " , %s=%d ", col, atoi(val));
            } else if (!strcmp(type, "float")) {
                if (str->len <= 0)
                    string_appendf(str, " %s=%f ", col, atof(val));
                else
                    string_appendf(str, " , %s=%f ", col, atof(val));
            } else if (!strcmp(type, "point")) {
                mutil_real_escape_string_nalloc(&esc, val, strlen(val));
                if (str->len <= 0)
                    string_appendf(str, " %s= point '%s' ", col, esc);
                else
                    string_appendf(str, " , %s= point '%s' ", col, esc);
                free(esc);
            }
        } else if (require && !strcmp(require, "true")) {
            return nerr_raise(NERR_ASSERT, "require %s %s", name, type);
        }
        
        node = hdf_obj_next(node);
    }

    if (str->len <= 0)
        return nerr_raise(NERR_ASSERT, "str len 0");

    return STATUS_OK;
}

NEOERR* mcs_build_querycond(HDF *data, HDF *node, STRING *str, char *defstr)
{
    if (!data || !node || !str) return nerr_raise(NERR_ASSERT, "param err");
    
    char *name, *col, *val, *esc, *require, *type;

    node = hdf_obj_child(node);
    
    while (node) {
        name = hdf_obj_name(node);
        col = hdf_obj_value(node);
        val = hdf_get_value(data, name, NULL);
        require = mutil_obj_attr(node, "require");
        type = mutil_obj_attr(node, "type");
        if (val && *val) {
            if (type == NULL || strcmp(type, "int")) {
                mutil_real_escape_string_nalloc(&esc, val, strlen(val));
                if (str->len <= 0)
                    string_appendf(str, " %s '%s' ", col, esc);
                else
                    string_appendf(str, " AND %s '%s' ", col, esc);
                free(esc);
            } else {
                if (str->len <= 0)
                    string_appendf(str, " %s %d ", col, atoi(val));
                else
                    string_appendf(str, " AND %s %d ", col, atoi(val));
            }
        } else if (require && !strcmp(require, "true")) {
            return nerr_raise(NERR_ASSERT, "require %s %s", name, type);
        }
        
        node = hdf_obj_next(node);
    }
    
    if (str->len <= 0 && defstr) string_append(str, defstr);

    return STATUS_OK;
}

NEOERR* mcs_build_incol(HDF *data, HDF *node, STRING *str)
{
    if (!data || !node || !str) return nerr_raise(NERR_ASSERT, "param err");
    
    char *name, *col, *val, *esc, *require, *clen, *type;
    STRING sa, sb;
    string_init(&sa);
    string_init(&sb);

    node = hdf_obj_child(node);
    
    while (node) {
        name = hdf_obj_name(node);
        col = hdf_obj_value(node);
        val = hdf_get_value(data, name, NULL);
        require = mutil_obj_attr(node, "require");
        clen = mutil_obj_attr(node, "maxlen");
        type = mutil_obj_attr(node, "type");
        if (val && *val) {
            if (type == NULL || !strcmp(type, "str")) {
                mutil_real_escape_string_nalloc(&esc, val, strlen(val));
                if (sa.len <= 0) {
                    string_appendf(&sa, " (%s ", col);
                    if (clen)
                        string_appendf(&sb, " VALUES ('%s'::varchar(%d) ",
                                       esc, atoi(clen));
                    else
                        string_appendf(&sb, " VALUES ('%s' ", esc);
                } else {
                    string_appendf(&sa, ", %s ", col);
                    if (clen)
                        string_appendf(&sb, ", '%s'::varchar(%d) ",
                                       esc, atoi(clen));
                    else
                        string_appendf(&sb, ", '%s' ", esc);
                }
                free(esc);
            } else if (!strcmp(type, "int")) {
                if (sa.len <= 0) {
                    string_appendf(&sa, " (%s ", col);
                    string_appendf(&sb, " VALUES (%d ", atoi(val));
                } else {
                    string_appendf(&sa, ", %s ", col);
                    string_appendf(&sb, ", %d ", atoi(val));
                }
            } else if (!strcmp(type, "float")) {
                if (sa.len <= 0) {
                    string_appendf(&sa, " (%s ", col);
                    string_appendf(&sb, " VALUES (%f ", atof(val));
                } else {
                    string_appendf(&sa, ", %s ", col);
                    string_appendf(&sb, ", %f ", atof(val));
                }
            } else if (!strcmp(type, "point")) {
                mutil_real_escape_string_nalloc(&esc, val, strlen(val));
                if (sa.len <= 0) {
                    string_appendf(&sa, " (%s ", col);
                    string_appendf(&sb, " VALUES (point '%s' ", esc);
                } else {
                    string_appendf(&sa, ", %s ", col);
                    string_appendf(&sb, ", point '%s' ", esc);
                }
                free(esc);
            }
        } else if (require && !strcmp(require, "true")) {
            string_clear(&sa);
            string_clear(&sb);
            return nerr_raise(NERR_ASSERT, "require %s %s", name, type);
        }
        
        node = hdf_obj_next(node);
    }

    if (sa.len <= 0) {
        string_clear(&sa);
        string_clear(&sb);
        return nerr_raise(NERR_ASSERT, "str len 0");
    }

    string_appendf(str, "%s)  %s)", sa.buf, sb.buf);

    string_clear(&sa);
    string_clear(&sb);
    
    return STATUS_OK;
}

void mcs_html_escape(HDF *node, char *name)
{
    if (!node || !name) return;
    char *s, *os;
    
    while (node) {
        s = hdf_get_value(node, name, NULL);
        if (s) {
            neos_html_escape(s, strlen(s), &os);
            hdf_set_value(node, name, os);
            free(os);
        }
        
        node = hdf_obj_next(node);
    }
}

NEOERR* mcs_err_valid(NEOERR *err)
{
    NEOERR *r = err;

    while (r && r != INTERNAL_ERR) {
        if (r->error != NERR_PASS) break;
        r = r->next;
    }
    
    return r;
}
