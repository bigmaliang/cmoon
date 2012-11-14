#include "mheads.h"

HDF *g_cfg = NULL;

struct elist {
    char *name;
    char *addr;
    struct elist* next;
};

struct elist *list = NULL;

int main()
{
    HASH *etbl = NULL;
    void *p;

    hash_init(&etbl, hash_str_hash, hash_str_comp);

    list = calloc(1, sizeof(struct elist));

    list->name = "ape_ext_a";
    list->addr = "ape_ext_a";
    list->next = NULL;

    struct elist *nl = calloc(1, sizeof(struct elist));
    nl->name = "ape_ext_b";
    nl->addr = "ape_ext_b";
    nl->next = list;

    list = nl;
    
    hash_insert(etbl, "ape_ext_a", "ape_ext_a");
    hash_insert(etbl, "ape_ext_b", "ape_ext_b");

    unsigned long elapsed;
    mtimer_start();
    for (int i = 0; i < 100000000; i++) {
#if 0
        p = hash_lookup(etbl, "ape_ext_b");
        if (!p) {
            printf("error");
            break;
        }
#endif
        nl = list;
        while (nl) {
            if (!strcmp(nl->name, "ape_ext_a"))
                break;
            nl = nl->next;
        }
        if (!nl) {
            printf("error");
            break;
        }
    }
    mtimer_stop(NULL);

    return 0;
}
