#ifndef __MNUM_H__
#define __MNUM_H__

#include "mheads.h"

__BEGIN_DECLS

enum user_status
{
	MNUM_USER_FRESH = 0,
	MNUM_USER_RLSED,
	MNUM_USER_CFMED
};

#define MIN_USER_NUM	998

#define LMT_CLI_REGIST	400
#define LMT_CLI_LOGIN	400
#define DIV_USER_TB	100

#define DFT_NUM_PERPAGE	15
#define DFT_PAGE_NUM	1

#define LEN_INT		16		// int max length
#define LEN_LONG	20
#define LEN_DT		11		// date length 2008-01-01
#define LEN_TM		25		// time 2008-01-01 12:12:43
#define LEN_TM_GMT	32		/* Wdy, DD-Mon-YYYY HH:MM:SS GMT */
#define LEN_IP		16		// IP length 202.202.202.202
#define LEN_TB		64		/* table name length */
#define LEN_CK		32		/* user login cookie length */
#define LEN_MMC_KEY	256		/* memcached key length */
#define LEN_LT		43
#define LEN_ST		65
#define LEN_SM		257
#define LEN_MD		1025
#define LEN_ML		10001		/* 2000*5+1 */
#define LEN_LN		65537
#define LEN_FN		1024		/* LEN_FILENAME */
#define LEN_LINE	(1024*100)
#define LEN_SQL		1024

#define RET_USER_LOGIN	0
#define RET_USER_NLOGIN	1

#define RET_RBTOP_OK		0
#define RET_RBTOP_INITE		11
#define RET_RBTOP_INPUTE	12
#define RET_RBTOP_HDFNINIT	21
#define RET_RBTOP_DBNINIT	31
#define RET_RBTOP_DBINTRANS	32
#define RET_RBTOP_SELECTE	41
#define RET_RBTOP_INSERTE	42
#define RET_RBTOP_UPDATEE	43
#define RET_RBTOP_DELETEE	44

#define RET_RBTOP_MEMALLOCE	100

#define RET_RBTOP_ERROR		1023
#define RET_RBTOP_MAX_M		1024

#define ONE_MINUTE	60
#define FIVE_MINUTE	(5*60)
#define HALF_HOUR	(30*60)
#define ONE_HOUR	(60*60)
#define ONE_DAY		(60*60*24)
#define ONE_WEEK	(7*60*60*24)
#define ONE_MONTH	(30*60*60*24)

__END_DECLS
#endif	/* __MNUM_H__ */
