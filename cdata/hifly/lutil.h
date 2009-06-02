#ifndef __LUTIL_H__
#define __LUTIL_H__
#include "mheads.h"

__BEGIN_DECLS

bool lutil_is_userdata_key(char *key);
bool lutil_user_has_power(const char *user, const char *pass);

__END_DECLS
#endif	/* __LUTIL_H__ */
