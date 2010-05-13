#include <sys/types.h>		/* for size_t */
#include <stdint.h>		/* for [u]int*_t */
#include <stdbool.h>		/* bool */
#include <stdlib.h>		/* for malloc() */
#include <string.h>		/* for memcpy()/memcmp() */
#include <stdio.h>		/* snprintf() */
#include <unistd.h>		/* usleep() */
#include "net-const.h"
#include "common.h"
#include "packet.h"
#include "log.h"
#include "queue.h"
#include "reply.h"

#include "mheads.h"
#include "ClearSilver.h"

#include "smsalarm.h"

void smsalarm_msg(char *msg)
{
	mdb_conn *db;
	//char content[100];
	int ret;

	if (mdb_init(&db, SMSA_DB_SN) != RET_RBTOP_OK) {
		wlog("init alarm db err %s\n", mdb_get_errmsg(db));
		return;
	}

	HDF *node = hdf_get_obj(g_cfg, SMSA_CFG_PATH".leader");
	if (node != NULL) node = hdf_obj_child(node);

	while (node != NULL) {
		//memset(content, 0x0, sizeof(content));
		ret = mdb_exec(db, NULL,
					   "INSERT INTO monitor_smssend (smsSendTo, smsContent) VALUES ('%s', '%s')",
					   NULL, hdf_obj_value(node), msg);
		if (ret != MDB_ERR_NONE) {
			wlog("exec %s failure %s\n", msg, mdb_get_errmsg(db));
		} else {
			wlog("%s alarm ok\n", msg);
		}
		
		node = hdf_obj_next(node);
	}

	mdb_destroy(db);
}

void smsalarm_msgf(char *fmt, ...)
{
	char *msg;
	va_list ap;

	va_start(ap, fmt);
	msg = vsprintf_alloc(fmt, ap);
	va_end(ap);
	if (msg == NULL) return ;

	smsalarm_msg(msg);
	free(msg);
}
