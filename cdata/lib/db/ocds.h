#ifndef __OCDS_H__
#define __OCDS_H__

#include "fheads.h"

#define CDS_DOMAIN_NUM	7

int cds_parse_key(char *key, ULIST **list);
int cds_parse_domain(char *domain, ULIST **list);
int cds_add_udp_server(nmdb_t *db, char *domain);
int cds_get_data(HDF *hdf, char *key, char *domain, char *hdfkey, fdb_t *fdb);
int cds_store_increment(fdb_t *fdb, char *key, char *val);

#endif	/* __OCDS_H__ */
