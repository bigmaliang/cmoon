/*
 * config parser(get cfg item from shm on non-first call for speed reason)
 * 1, one line per config item
 * 2, line start with '#' will be ommit
 *
 */
#ifndef __MCONFIG_H__
#define __MCONFIG_H__

#include "mheads.h"

__BEGIN_DECLS

#ifdef RELEASE
#define SITE_DFT_CONFIG	"/usr/local/moon/site.conf"
#else
#define SITE_DFT_CONFIG	"/home/bigml/web/moon/site.conf"
#endif
#define CFG_SHMEM_KEY	0x00054110
#define SHM_MODE		IPC_EXCL|0666
#define SHM_ADDR		0x00000000

#define CFG_ITEM_NUM	512
#define CFG_ITEM_LEN	512
#define CFG_DIVIDER		"'"

/*
 * mcfg_xxx serials function is deprecated,
 * please use mconfig_xxx instead.
 *
 * except you need to change config value in runtime
 */
void mcfg_init(const char *fn);
char* mcfg_getvalue(const char *key, const char *def);
int	mcfg_getintvalue(const char *key);
int	mcfg_getpos(const char *key);
bool mcfg_additem(const char *key, const char *value);
bool mcfg_setitem(const char *key, const char *value);
void mcfg_leave(void);

NEOERR* mconfig_parse_file(const char *file, HDF **cfg);
void mconfig_cleanup(HDF **config);

__END_DECLS
#endif	/* __MCONFIG_H__ */
