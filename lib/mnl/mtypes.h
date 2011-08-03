#ifndef __MTYPES_H__
#define __MTYPES_H__

#include "mheads.h"

__BEGIN_DECLS

typedef struct _nav {
    char name[LEN_ST];
    char href[LEN_ST];
    char title[LEN_SM];
    char target[LEN_SM];
} anchor_t;

anchor_t* anchor_new(char *name, char *href,
                     char *title, char *target);
void anchor_del(anchor_t *anc);

__END_DECLS
#endif    /* __MTYPES_H__ */
