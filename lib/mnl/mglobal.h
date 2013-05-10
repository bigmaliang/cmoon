#ifndef __MGLOBAL_H__
#define __MGLOBAL_H__
#include "mheads.h"

__BEGIN_DECLS

/* need to declare by application */
extern HDF *g_cfg;              /* global config  */
extern HASH *g_datah;           /* global data e.g. InConfigRend ... */

/* use interally(don't need user declare) */
extern unsigned long elapsed;   /* mtimer.c */

__END_DECLS
#endif    /* __MGLOBAL_H__ */
