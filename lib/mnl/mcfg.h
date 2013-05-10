#ifndef __MCFG_H__
#define __MCFG_H__

#include "mheads.h"

__BEGIN_DECLS

#define DFT_NUM_PERPAGE 15
#define DFT_PAGE_NUM    1

#define QR_NUM_MAX      50
#define IMAGE_MD5_SIZE  (1024*1024)

#define LEN_INT     16        // int max length
#define LEN_LONG    24
#define LEN_DT      11        // date length 2008-01-01
#define LEN_TM      25        // time 2008-01-01 12:12:43
#define LEN_TM_GMT  32        /* Wdy, DD-Mon-YYYY HH:MM:SS GMT */
#define LEN_IP      16        // IP length 202.202.202.202
#define LEN_MD5     33        /* 098f6bcd4621d373cade4e832627b4f6\0 */
#define LEN_TB      64        /* table name length */
#define LEN_CK      32        /* user login cookie length */
#define LEN_LT      43
#define LEN_ST      64
#define LEN_SM      256
#define LEN_MD      1024
#define LEN_ML      10001        /* 2000*5+1 */
#define LEN_LN      65536

#define LEN_FN      _POSIX_PATH_MAX        /* LEN_FILENAME */
#define LEN_URL     _POSIX_PATH_MAX
#define LEN_LINE    (1024*100)
#define LEN_SQL     1024
#define LEN_HDF_KEY 64
#define LEN_HASH_KEY 128
#define LEN_MMC_KEY  256                    /* memcached key length */
#define LEN_NMDB_KEY 256
#define LEN_NMDB_VAL (1024*64)
#define BYTE_BSON_OID 12
#define LEN_BSON_OID  25         /* 12*2+1 */

#define ONE_MINUTE  60
#define FIVE_MINUTE (5*60)
#define HALF_HOUR   (30*60)
#define ONE_HOUR    (60*60)
#define ONE_DAY     (60*60*24)
#define ONE_WEEK    (7*60*60*24)
#define ONE_MONTH   (30*60*60*24)

#define PRE_HTTP        "HTTP"
#define PRE_CGI         "CGI"
#define PRE_COOKIE      "Cookie"
#define PRE_QUERY       "Query"
#define PRE_OUTPUT      "Output"
#define PRE_RESERVE     "Reserve"
#define PRE_TEMP        "Temp"

#define PRE_LAYOUT      "Layout"
#define PRE_INCLUDE     "Include"
#define PRE_DATASET     "Dataset"
#define PRE_VALUESET    "Valueset"
#define PRE_VALUEREP    "ValueReplace"
#define PRE_CONFIG      "Config"
#define PRE_SERVER      "Server"

#define PRE_ERRTRACE    PRE_OUTPUT".errtrace"
#define PRE_ERRMSG      PRE_OUTPUT".errmsg"
#define PRE_ERRCODE     PRE_OUTPUT".errcode"
#define PRE_SUCCESS     PRE_OUTPUT".success"

#define PRE_MMC_CLIENT   "Client"
#define PRE_MMC_LOGIN    "Login"
#define PRE_MMC_UNAME    "Uname"
#define PRE_MMC_UPASS    "Password"

#define PRE_CFG_MEMC     "Memcached"
#define PRE_CFG_EVT      "Mevent"

NEOERR* mcfg_parse_file(const char *file, HDF **cfg);
void mcfg_cleanup(HDF **config);

__END_DECLS
#endif    /* __MCFG_H__ */
