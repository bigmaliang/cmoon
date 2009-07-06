#ifndef __OACCOUNT_H__
#define __OACCOUNT_H__
#include "mheads.h"

__BEGIN_DECLS

int account_get_accounts(HDF *hdf, mdb_conn *conn, session_t *ses);
int account_add_partner(HDF *hdf, mdb_conn *conn, session_t *ses);

__END_DECLS
#endif /* __OACCOUNT_H__ */
