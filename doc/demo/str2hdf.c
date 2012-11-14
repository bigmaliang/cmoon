#include "mheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv, char **envp)
{
    char *s = "{\"a\": [{\"aaa\": 12}, 2], \"b\": \"xxx\"}";
    //char *s = "fuck";
    HDF *node;

    hdf_init(&node);

    mjson_string_to_hdf(hdf_get_obj(node, "s"), s);

    hdf_write_file(node, "x.hdf");
    hdf_destroy(&node);
    
    return 0;
}
