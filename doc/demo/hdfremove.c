#include "mheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv, char **envp)
{
    HDF *node, *cnode;

    hdf_init(&node);

    hdf_set_value(node, "foo.0.pic", "xxx.jpg");
    hdf_set_value(node, "bar.0.pic", "yyy.jpg");

    cnode = hdf_get_child(node, "foo");
    hdf_copy(node, "gifts.0", cnode);

    cnode = hdf_get_child(node, "bar");
    hdf_copy(node, "gifts.1", cnode);

    hdf_remove_tree(node, "bar");

    hdf_set_value(node, "zzzz", "4");

    hdf_destroy(&node);

    return 0;
}
