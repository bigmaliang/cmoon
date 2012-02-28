#include "mheads.h"

NEOERR* mcs_outputcb(void *ctx, char *s)
{
    printf ("%s", s);
    return STATUS_OK;
}

NEOERR* mcs_strcb(void *ctx, char *s)
{
    STRING *str = (STRING*)ctx;
    NEOERR *err;
    err = nerr_pass(string_append(str, s));
    return err;
}

NEOERR* mcs_str2file(STRING str, const char *file)
{
    if (file == NULL) return nerr_raise(NERR_ASSERT, "paramter null");
    
    FILE *fp = fopen(file, "w");
    if (!fp) return nerr_raise(NERR_IO, "unable to open %s for write", file);

    size_t ret = fwrite(str.buf, str.len, 1, fp);
    if (ret < 0) {
        fclose(fp);
        return nerr_raise(NERR_IO, "write str.buf to %s error", file);
    }
    
    fclose(fp);
    return STATUS_OK;
}

static NEOERR* _builtin_bitop_and(CSPARSE *parse, CS_FUNCTION *csf, CSARG *args,
                                  CSARG *result)
{
    NEOERR *err;
    long int n1 = 0;
    long int n2 = 0;

    result->op_type = CS_TYPE_NUM;
    result->n = 0;

    err = cs_arg_parse(parse, args, "ii", &n1, &n2);
    if (err) return nerr_pass(err);
    result->n = n1 & n2;

    return STATUS_OK;
}

static NEOERR* _builtin_bitop_or(CSPARSE *parse, CS_FUNCTION *csf, CSARG *args,
                                 CSARG *result)
{
    NEOERR *err;
    long int n1 = 0;
    long int n2 = 0;

    result->op_type = CS_TYPE_NUM;
    result->n = 0;

    err = cs_arg_parse(parse, args, "ii", &n1, &n2);
    if (err) return nerr_pass(err);
    result->n = n1 | n2;

    return STATUS_OK;
}

static NEOERR* _builtin_bitop_xor(CSPARSE *parse, CS_FUNCTION *csf, CSARG *args,
                                  CSARG *result)
{
    NEOERR *err;
    long int n1 = 0;
    long int n2 = 0;

    result->op_type = CS_TYPE_NUM;
    result->n = 0;

    err = cs_arg_parse(parse, args, "ii", &n1, &n2);
    if (err) return nerr_pass(err);
    result->n = n1 & ~n2;

    return STATUS_OK;
}

NEOERR* mcs_register_bitop_functions(CSPARSE *cs)
{
    NEOERR *err;
    
    err = cs_register_function(cs, "bitop.and", 2, _builtin_bitop_and);
    if (err != STATUS_OK) return nerr_pass(err);
    cs_register_function(cs, "bitop.or", 2, _builtin_bitop_or);
    if (err != STATUS_OK) return nerr_pass(err);
    cs_register_function(cs, "bitop.xor", 2, _builtin_bitop_xor);
    if (err != STATUS_OK) return nerr_pass(err);

    return STATUS_OK;
}

NEOERR* mcs_register_mkd_functions(CSPARSE *cs)
{
    return nerr_pass(cs_register_esc_strfunc(cs, "mkd.escape", mkd_esc_str));
}

NEOERR* mcs_register_upload_parse_cb(CGI *cgi, void *rock)
{
    NEOERR *err;
    
    err = cgi_register_parse_cb(cgi, "POST", "application/x-www-form-urlencoded",
                                rock, mhttp_upload_parse_cb);
	if (err != STATUS_OK) return nerr_pass(err);

    err = cgi_register_parse_cb(cgi, "POST", "multipart/form-data",
                                rock, mhttp_upload_parse_cb);
	if (err != STATUS_OK) return nerr_pass(err);

    err = cgi_register_parse_cb(cgi, "PUT", "*", rock, mhttp_upload_parse_cb);
    return nerr_pass(err);
}


unsigned int mcs_get_uint_value(HDF *hdf, const char *name, unsigned int defval)
{
    char *val, *n;
    unsigned int v;
    
    val = hdf_get_value(hdf, name, NULL);
    if (val) {
        v = strtoul(val, &n, 10);
        if (val == n) v = defval;
        return v;
    }
    return defval;
}

float mcs_get_float_value(HDF *hdf, const char *name, float defval)
{
    char *val, *n;
    float v;
    
    val = hdf_get_value(hdf, name, NULL);
    if (val) {
        v = strtof(val, &n);
        if (val == n) v = defval;
        return v;
    }
    return defval;
}

NEOERR* mcs_set_uint_value(HDF *hdf, const char *name, unsigned int value)
{
  char buf[256];

  snprintf(buf, sizeof(buf), "%u", value);
  return nerr_pass(hdf_set_value(hdf, name, buf));
}

NEOERR* mcs_set_float_value(HDF *hdf, const char *name, float value)
{
  char buf[256];

  snprintf(buf, sizeof(buf), "%f", value);
  return nerr_pass(hdf_set_value(hdf, name, buf));
}

char* mcs_hdf_attr(HDF *hdf, char *name, char*key)
{
    if (hdf == NULL || name == NULL || key == NULL)
        return NULL;
    
    HDF_ATTR *attr = hdf_get_attr(hdf, name);
    while (attr != NULL) {
        if (!strcmp(attr->key, key)) {
            return attr->value;
        }
        attr = attr->next;
    }
    return NULL;
}
char* mcs_obj_attr(HDF *hdf, char*key)
{
    if (hdf == NULL || key == NULL)
        return NULL;
    
    HDF_ATTR *attr = hdf_obj_attr(hdf);
    while (attr != NULL) {
        if (!strcmp(attr->key, key)) {
            return attr->value;
        }
        attr = attr->next;
    }
    return NULL;
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
