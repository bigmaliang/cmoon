#include "mheads.h"
#include "lheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv, char **envp)
{
    char *s = "{\"a\": [{\"aaa\": 12}, 2], \"b\": \"xxx\"}";
    //char *s = "fuck";
    HDF *node;

    hdf_init(&node);

    hdf_set_value(node, "s", s);

    mjson_str2hdf(hdf_get_obj(node, "s"), NULL);

    hdf_write_file(node, "x.hdf");
    hdf_destroy(&node);
    
    return 0;
}
