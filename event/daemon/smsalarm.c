#include <sys/types.h>		/* for size_t */
#include <stdint.h>		/* for [u]int*_t */
#include <stdbool.h>		/* bool */
#include <stdlib.h>		/* for malloc() */
#include <string.h>		/* for memcpy()/memcmp() */
#include <stdio.h>		/* snprintf() */
#include <unistd.h>		/* usleep() */
#include "net-const.h"
#include "common.h"
#include "data.h"
#include "packet.h"
#include "log.h"
#include "dtrace.h"
#include "queue.h"
#include "reply.h"

#include "ClearSilver.h"

#include "fdb.h"
#include "smsalarm.h"

void smsalarm_msg(char *msg)
{
	fdb_t *db;
	//char content[100];
	int ret;
	
	ret = fdb_init_long(&db, SMSA_CFG_IP, SMSA_CFG_USER,
						SMSA_CFG_PASS, SMSA_CFG_NAME, SMSA_CFG_PORT);
	if (ret != RET_DBOP_OK) {
		wlog("init alarm db err %s\n", fdb_error(db));
		return;
	}

	HDF *node = hdf_get_obj(g_cfg, SMSA_CFG_PATH".leader");
	if (node != NULL) node = hdf_obj_child(node);

	while (node != NULL) {
		//memset(content, 0x0, sizeof(content));
		memset(db->sql, 0x0, sizeof(db->sql));
		
		//snprintf(content, sizeof(content), "%s|%s", hdf_obj_value(node), msg);
		snprintf(db->sql, sizeof(db->sql),
				 "INSERT INTO monitor_smssend (smsSendTo, smsContent) VALUES ('%s', '%s')",
				 hdf_obj_value(node), msg);
		ret = fdb_exec(db);
		if (ret != RET_DBOP_OK) {
			wlog("exec %s failure %s\n", msg, fdb_error(db));
		} else {
			wlog("%s alarm ok\n", msg);
		}
		
		node = hdf_obj_next(node);
	}

	fdb_free(&db);
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
