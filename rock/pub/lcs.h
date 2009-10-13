#ifndef __LCS_H__
#define __LCS_H__
#include "mheads.h"

__BEGIN_DECLS

int lcs_hdf2list(HDF *hdf, char *prefix, void (*hdf2item)(HDF *hdf, void **item),
                 ULIST **ul);
int lcs_list2hdf(ULIST *ul, char *prefix,
                 void (*item2hdf)(void *item, char *pre, HDF *hdf),
                 HDF *hdf);

__END_DECLS
#endif	/* __LCS_H__ */
