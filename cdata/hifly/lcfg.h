#ifndef __FCONFIG_H__
#define __FCONFIG_H__

#define POST_INCREMENT	"increment"
#define POST_TIMESTAMP	"timestamp"

#define CFG_DB			"Db"
#define CFG_NMDB		"Nmdb"
#define CFG_TABLE		"Tables"

#ifdef RELEASE
#define PATH_SITE	"/data/www/rtds/"
#else
#define PATH_SITE	"/usr/local/moon/cdata/"
#endif

#define CONFIG_FILE PATH_SITE"config.hdf"
#define HF_LOG_PATH	"/data/logs/cgi/"

#endif	/* __FCONFIG_H__ */
