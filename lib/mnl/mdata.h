#ifndef __MDATA_H__
#define __MDATA_H__

#include "mheads.h"

__BEGIN_DECLS

#ifndef USE_EVENT
#define MDATA_SET(conn, plugin, afrow, flag, sql_fmt, fmt, ...)	\
	mdb_exec(conn, afrow, sql_fmt, fmt, ##__VA_ARGS__)
#else
#define MDATA_SET(conn, plugin, afrow, flag, sql_fmt, fmt, ...)	\
	mdata_exec(plugin, afrow, flag, sql_fmt, fmt, ##__VA_ARGS__)
#endif

int mdata_exec(char *plugin, int *afrow, unsigned short flag,
			   const char *sql_fmt, const char *fmt, ...);

__END_DECLS
#endif	/* __MDATA_H__ */
