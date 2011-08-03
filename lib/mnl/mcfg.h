#ifndef __MCFG_H__
#define __MCFG_H__

#include "mheads.h"

__BEGIN_DECLS

#define PRE_CGI        "CGI"
#define PRE_COOKIE    "Cookie"
#define PRE_QUERY    "Query"
#define PRE_LAYOUT    "Layout"
#define PRE_OUTPUT    "Output"
#define PRE_INCLUDE    "Include"
#define PRE_CONFIG    "Config"
#define PRE_RESERVE    "Reserve"
#define PRE_ERRTRACE PRE_OUTPUT".errtrace"
#define PRE_ERRMSG   PRE_OUTPUT".errmsg"
#define PRE_ERRCODE  PRE_OUTPUT".errcode"
#define PRE_SUCCESS  PRE_OUTPUT".success"

#define PRE_MMC_CLIENT    "Client"
#define PRE_MMC_LOGIN    "Login"
#define PRE_MMC_UNAME    "Uname"
#define PRE_MMC_UPASS    "Password"

#define PRE_CFG_MEMC    "Memcached"
#define PRE_CFG_EVT        "Mevent"

__END_DECLS
#endif    /* __MCFG_H__ */
