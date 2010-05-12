#ifndef __LTYPES_H__
#define __LTYPES_H__

#include "mheads.h"

__BEGIN_DECLS

typedef struct _session {
	char *uname;
	time_t tm_cache_browser;
} session_t;

int session_init(HDF *hdf, HASH *dbh, session_t **ses);
void session_destroy(session_t **ses);

__END_DECLS
#endif	/* __LTYPES_H__ */
