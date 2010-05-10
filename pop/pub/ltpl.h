#ifndef __LTPL_H__
#define __LTPL_H__
#include "mheads.h"

__BEGIN_DECLS

int  ltpl_parse_dir(char *dir, HASH *outhash);
int  ltpl_init(HASH **tplh);
void ltpl_destroy(HASH *tplh);

__END_DECLS
#endif	/* __LTPL_H__ */
