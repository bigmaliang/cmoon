#ifndef __MJSON_H__
#define __MJSON_H__

#include "mheads.h"

__BEGIN_DECLS

void mjson_execute_hdf(HDF *hdf, char *cb, time_t second);
void mjson_output_hdf(HDF *hdf, time_t second);

__END_DECLS
#endif    /* __MJSON_H__ */
