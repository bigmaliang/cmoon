
#ifndef _PARSE_H
#define _PARSE_H

#include "req.h"

int parse_message(struct req_info *req,
		  const unsigned char *buf, size_t len);

#endif

