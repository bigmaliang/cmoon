#ifndef __LCFG_H__
#define __LCFG_H__

#include "mheads.h"

__BEGIN_DECLS

#define SITE_DOMAIN	"eol.com"
#define TC_ROOT		"/var/log/moon/rock/"

#ifdef RELEASE
#define PATH_SITE	"/usr/local/moon/www/"
#else
#define PATH_SITE	"/home/bigml/web/moon/"
#endif
#define SITE_CONFIG		PATH_SITE"site.conf"
#define PATH_DOC	PATH_SITE"rock/fly/"
#define PATH_JS		PATH_SITE"rock/fly/js/"
#define PATH_CGI	PATH_SITE"rock/run/"
#define PATH_TPL	PATH_SITE"rock/tpl/"
#define PATH_MTLS	PATH_SITE"rock/mtls/"
#define PATH_ADM_DOC	PATH_SITE"admin/htdocs/"
#define PATH_ADM_CGI	PATH_SITE"admin/cgi-bin/"
#define PATH_ADM_TPL	PATH_SITE"admin/tpl/"
#define PATH_ADM_MTLS	PATH_SITE"admin/mtls/"
#define F_TPL_LAYOUT	PATH_SITE"rock/tpl/layout.html"

#define CGI_RUN_DIR		"run"
#define URI_SPLITER		"/"

#define PRE_MMC_FILE	"File"
#define PRE_MMC_MEMBER	"Member"

#define NAV_NUM 4
extern anchor_t g_nav[NAV_NUM];

__END_DECLS
#endif	/* __LCFG_H__ */
