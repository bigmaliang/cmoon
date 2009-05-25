#include "mheads.h"
#include "lheads.h"

bool futil_is_userdata_key(char *key)
{
	char *p = key+strlen(key)-1;
	while (*p != '_' && p != key) {
		if (!isdigit(*p))
			return false;
		p--;
	}
	return true;
}
