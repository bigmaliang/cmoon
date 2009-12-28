#ifndef _HNPUB_H
#define _HNPUB_H

#include "main.h"
#include "cmd.h"
#include "raw.h"
#include "extend.h"
#include "channel.h"

void hn_senderr(callbackp *callbacki, char *code, char *msg);
int hn_isvaliduin(char *uin);

#endif
