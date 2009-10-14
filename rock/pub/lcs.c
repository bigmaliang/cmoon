#include "mheads.h"
#include "lheads.h"

int lcs_hdf2list(HDF *hdf, char *prefix, void (*hdf2item)(HDF *hdf, void **item),
                 ULIST **ul)
{
    if (hdf == NULL || prefix == NULL || hdf2item == NULL)
        return RET_RBTOP_INPUTE;

    char tok[LEN_MD];
    snprintf(tok, sizeof(tok), "%s.0", prefix);
    
    HDF *node = hdf_get_obj(hdf, tok);
    if (node == NULL) return RET_RBTOP_INPUTE;
    
    if (*ul == NULL) {
        uListInit(ul, 0, 0);
    }

    void *item;
    while (node != NULL) {
        hdf2item(node, &item);
        uListAppend(*ul, item);

		node = hdf_obj_next(node);
    }

    return RET_RBTOP_OK;
}

int lcs_list2hdf(ULIST *ul, char *prefix,
                 void (*item2hdf)(void *item, char *pre, HDF *hdf),
                 HDF *hdf)
{
    if (ul == NULL || prefix == NULL || item2hdf == NULL || hdf == NULL)
        return RET_RBTOP_INPUTE;

    char tok[LEN_MD];
    
    ITERATE_MLIST(ul) {
        snprintf(tok, sizeof(tok), "%s.%d", prefix, t_rsv_i);
        item2hdf(ul->items[t_rsv_i], tok, hdf);
    }
    
    return RET_RBTOP_OK;
}
