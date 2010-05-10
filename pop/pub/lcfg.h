#ifndef __LCFG_H__
#define __LCFG_H__

#include "mheads.h"

__BEGIN_DECLS

#define SITE_DOMAIN	"bmoon.com"
#define TC_ROOT		"/var/log/moon/pop/"

#ifdef RELEASE
#define PATH_SITE	"/usr/local/moon/pop/"
#else
#define PATH_SITE	"/home/bigml/web/moon/pop/"
#endif
#define SITE_CONFIG	PATH_SITE"config.hdf"
#define PATH_DOC	PATH_SITE"fly/"
#define PATH_JS		PATH_SITE"fly/js/"
#define PATH_CGI	PATH_SITE"run/"
#define PATH_TPL	PATH_SITE"tpl/"
#define PATH_PAGER	PATH_SITE"pager/"
#define PATH_MTLS	PATH_SITE"mtls/"
#define F_TPL_LAYOUT PATH_SITE"tpl/layout.htm"

#define CGI_RUN_DIR		"run"
#define URI_SPLITER		"/"

#define PRE_OUT_TPL		PRE_OUTPUT".tpl" /* ltpl_prepare_rend() */
#define PRE_OUT_UIPLUG	PRE_OUTPUT".Uiplug"

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
#define PRE_CFG_UIPLUG		"Uiplug"

#define EVT_PLUGIN_SYS		"db_sys"
#define EVT_PLUGIN_USER		"db_user"
#define EVT_PLUGIN_TJT		"db_tjt"

__END_DECLS
#endif	/* __LCFG_H__ */
