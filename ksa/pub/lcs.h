#ifndef __LCS_H__
#define __LCS_H__
#include "mheads.h"

__BEGIN_DECLS

int lcs_set_layout_infoa(HDF *hdf, const char *title, anchor_t *crumbs,
						 anchor_t *g_nav, int navnum);

__END_DECLS
#endif	/* __LCS_H__ */
