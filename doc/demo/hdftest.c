#include "mheads.h"
#include "lheads.h"

HDF *g_cfg = NULL;
HASH *g_datah = NULL;

#define NUM_TOTAL_TEST 100000

#define VERBERSE 0

char *bdatam = "{'number': 5.05, 'boolean': FALSE, 'array': ['test', 'benchmark']}";
//char *bdatam = "[10, 11]";

int main(int argc, char **argv, char **envp)
{
    struct json_object *jso;
    HDF *node;
    bson *doc;
    NEOERR *err;
    
    mtc_init(TC_ROOT"hdftest");

    mtimer_start();
    for (int i = 0; i < NUM_TOTAL_TEST; i++) {
        hdf_init(&node);
        err = mjson_string_to_hdf(node, bdatam);
        OUTPUT_NOK(err);
        if (VERBERSE) hdf_dump(node, NULL);
        hdf_destroy(&node);
    }
    mtimer_stop("json => hdf");
    mtc_foo("%d per second", (int)(NUM_TOTAL_TEST/(elapsed/1000000.0f)));

    mtimer_start();
    for (int i = 0; i < NUM_TOTAL_TEST; i++) {
        doc = mbson_new_from_string(bdatam, true);
        if (!doc) printf("bson error");
        if (VERBERSE) printf("%s\n", mbson_string(doc));
        bson_free(doc);
    }
    mtimer_stop("json => bson");
    mtc_foo("%d per second", (int)(NUM_TOTAL_TEST/(elapsed/1000000.0f)));


    

    hdf_init(&node);
    hdf_set_value(node, NULL, bdatam);
    err = mjson_string_to_hdf(node, bdatam);
    OUTPUT_NOK(err);
    
    mtimer_start();
    for (int i = 0; i < NUM_TOTAL_TEST; i++) {
        err = mjson_import_from_hdf(node, &jso);
        OUTPUT_NOK(err);
        if (VERBERSE) printf("%s\n", json_object_to_json_string(jso));
        json_object_put(jso);
    }
    mtimer_stop("hdf => json");
    mtc_foo("%d per second", (int)(NUM_TOTAL_TEST/(elapsed/1000000.0f)));

    mtimer_start();
    for (int i = 0; i < NUM_TOTAL_TEST; i++) {
        err = mbson_import_from_hdf(node, &doc, true);
        OUTPUT_NOK(err);
        if (VERBERSE) printf("%s\n", mbson_string(doc));
        bson_free(doc);
    }
    mtimer_stop("hdf => bson");
    mtc_foo("%d per second", (int)(NUM_TOTAL_TEST/(elapsed/1000000.0f)));





    doc = mbson_new_from_string(bdatam, true);
    if (!doc) printf("bson error");

    mtimer_start();
    for (int i = 0; i < NUM_TOTAL_TEST; i++) {
        char *s = mbson_string(doc);
        if (!s) printf("bson->string error");
        if (VERBERSE) printf("%s\n", s);
        free(s);
    }
    mtimer_stop("bson => json");
    mtc_foo("%d per second", (int)(NUM_TOTAL_TEST/(elapsed/1000000.0f)));

    mtimer_start();
    for (int i = 0; i < NUM_TOTAL_TEST; i++) {
        hdf_init(&node);
        err = mbson_export_to_hdf(node, doc, false, false);
        OUTPUT_NOK(err);
        if (VERBERSE) hdf_dump(node, NULL);
        hdf_destroy(&node);
    }
    mtimer_stop("bson => hdf");
    mtc_foo("%d per second", (int)(NUM_TOTAL_TEST/(elapsed/1000000.0f)));
    bson_free(doc);

    return 0;
}
    
