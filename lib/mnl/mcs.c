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

void mcs_rand_string(char *s, int max)
{
    int size;
    int x = 0;

    size = neo_rand(max-1);
    for (x = 0; x < size; x++)
    {
        s[x] = (char)(65 + neo_rand(90-65));
    }
    s[x] = '\0';
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

void mcs_hdf_escape_val(HDF *hdf)
{
    char *esc = NULL, *val = NULL;
    HDF *child, *next;
    
    if (!hdf) return;

    val = hdf_obj_value(hdf);
    if (val) {
        if (mutil_real_escape_string_nalloc(&esc, val, strlen(val))) {
            hdf_set_value(hdf, NULL, esc);
            free(esc);
        }
    }

    child = hdf_obj_child(hdf);
    if (child) {
        mcs_hdf_escape_val(child);
    }

    next = hdf_obj_next(hdf);
    while (next) {
        mcs_hdf_escape_val(next);
        next = hdf_obj_next(next);
    }
}
