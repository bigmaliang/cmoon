#ifndef __LCFG_H__
#define __LCFG_H__

#include "mheads.h"

__BEGIN_DECLS

#define SITE_DOMAIN	"eol.com"
#define IMG_DOMAIN	"http://img.eol.com"
#define TC_ROOT		"/var/log/moon/rock/"
#define IMG_ROOT	"/data/pic/"

#define IMG_ORI	"ori"
#define IMG_XXS	"24x24"
#define IMG_XS	"48x48"
#define IMG_S	"120x120"
#define IMG_M	"250x250"
#define IMG_L	"800X600"
#define IMG_XL	"1024X768"
#define IMG_XXL	"1440x900"

#ifdef RELEASE
#define PATH_SITE	"/usr/local/moon/rock/"
#else
#define PATH_SITE	"/home/bigml/web/moon/rock/"
#endif
#define SITE_CONFIG	PATH_SITE"config.hdf"
#define PATH_DOC	PATH_SITE"fly/"
#define PATH_JS		PATH_SITE"fly/js/"
#define PATH_CGI	PATH_SITE"run/"
#define PATH_TPL	PATH_SITE"tpl/"
#define PATH_PAGER	PATH_SITE"pager/"
#define PATH_MTLS	PATH_SITE"mtls/"
#define F_TPL_LAYOUT PATH_SITE"tpl/layout.html"

#define CGI_RUN_DIR		"run"
#define URI_SPLITER		"/"

#define PRE_OUT_TPL		PRE_OUTPUT".tpl" /* ltpl_prepare_rend() */

#define PRE_REQ_URI		PRE_CGI".ScriptName" 		/* lutil_file_access() */
#define PRE_REQ_URI_RW	PRE_QUERY".ScriptName"		/* lutil_file_access_rewrited() */
#define PRE_REQ_AJAX_FN	PRE_QUERY".JsonCallback"

#define PRE_RSV_FILE		PRE_RESERVE".RequestFile"	/* lutil_get_data_handler() */
#define PRE_RSV_REQ_TYPE	PRE_RESERVE".RequestType"
#define PRE_RSV_DATAER		PRE_RESERVE".Dataer"
#define PRE_RSV_RENDER		PRE_RESERVE".Render"

#define PRE_MMC_FILE		"File"
#define PRE_MMC_MEMBER		"Member"
#define PRE_MMC_GROUP		"Group"
#define PRE_MMC_TJT			"Tjt"
#define PRE_MMC_COUNT		"Ttnum"

#define PRE_CFG_OUTPUT		"Output"
#define PRE_CFG_LAYOUT		"Layout"
#define PRE_CFG_DATASET		"Dataset"
#define PRE_CFG_DATAER		"data_geter"
#define PRE_CFG_FILECACHE	"FileCache"

#define EVT_PLUGIN_SYS		"db_sys"
#define EVT_PLUGIN_USER		"db_user"
#define EVT_PLUGIN_TJT		"db_tjt"

__END_DECLS
#endif	/* __LCFG_H__ */
