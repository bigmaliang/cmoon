#ifndef __LCFG_H__
#define __LCFG_H__

#include "mheads.h"

__BEGIN_DECLS

#define SITE_DOMAIN	"kosherall.com"
#define TC_ROOT		"/tmp/kosherall/"

#ifdef RELEASE
/* #define PATH_SITE	"/usr/local/moon/www/" */
#define PATH_SITE	"/home/bigml/web/moon/"
#define SITE_CONFIG	"/home/kosheeral/www/site.conf"
#else
#define PATH_SITE	"/home/bigml/web/moon/"
#define SITE_CONFIG	"/home/bigml/web/moon/ksa/site.conf"
#endif
#define PATH_FRT_DOC	PATH_SITE"ksa/htdocs/"
#define PATH_FRT_CGI	PATH_SITE"ksa/cgi-bin/"
#define PATH_FRT_TPL	PATH_SITE"ksa/tpl/"
#define PATH_FRT_MTLS	PATH_SITE"ksa/mtls/"
#define PATH_ADM_DOC	PATH_SITE"admin/htdocs/"
#define PATH_ADM_CGI	PATH_SITE"admin/cgi-bin/"
#define PATH_ADM_TPL	PATH_SITE"admin/tpl/"
#define PATH_ADM_MTLS	PATH_SITE"admin/mtls/"
#define F_TPL_LAYOUT	PATH_SITE"ksa/tpl/layout.html"

#define NAV_NUM 6
extern anchor_t g_nav[NAV_NUM];

__END_DECLS
#endif	/* __LCFG_H__ */
