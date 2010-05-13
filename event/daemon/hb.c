/*
 * 事件中心心跳检测报警程序, 需配合 crontab 定时拉起使用
 * PATH/hb 192.168.1.15 26000 uic ../daemon/mevent.hdf > /tmp/meventhb.log
 * 每次检查一个后端插件的工作情况
 *
 * 正常情况下 输出 后端插件的 stats 统计信息
 * 处理错误 0x800 时 短信上报
 * 网络,客户端错误时  短信上报 trigger return
 */
#include <stdio.h>		/* printf() */
#include <unistd.h>		/* malloc(), fork() and getopt() */
#include <stdlib.h>		/* atoi() */
#include <sys/types.h>	/* for pid_t */
#include <string.h>		/* for strcpy() and strlen() */
#include <pthread.h>	/* for pthread_t */

#include "mevent.h"		/* api's mevent.h */

#include "common.h"
#include "net.h"
#include "net-const.h"
#include "log.h"
#include "stats.h"
#include "config.h"
#include "smsalarm.h"

int main(int argc, char *argv[])
{
	int ret;

	if (argc != 5) {
		printf("Usage: %s HOST PORT EVENT_NAME CONFIG_FILE\n", argv[0]);
		return 1;
	}

	settings.smsalarm = 1;
	//settings.logfname = "-";
	log_init();
	if (config_parse_file(argv[4], &g_cfg) != 1) {
		printf("parse config file %s failure", argv[3]);
			return 1;
	}
	
	mevent_t *evt = mevent_init();
	if (evt == NULL) {
		printf("init error\n");
		SMS_ALARM("mevent_init error");
		return 1;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 800000;
	
	mevent_add_udp_server(evt, argv[1], atoi(argv[2]), NULL, &tv);
	mevent_chose_plugin(evt, argv[3], REQ_CMD_STATS, FLAGS_SYNC);
	ret = mevent_trigger(evt);
	if (PROCESS_OK(ret)) {
		hdf_dump(evt->hdfrcv, NULL);
	} else {
		int try = 0;
		
	redo:
		sleep(10);
		ret = mevent_trigger(evt);
		if (PROCESS_NOK(ret) && try < 3) {
			try++;
			goto redo;
		}

		if (PROCESS_NOK(ret) && try >= 3) {
			printf("process failure %d\n", ret);
			SMS_ALARM("process failure %d, restarted", ret);
			system("killall -9 mevent && sleep 2 && /usr/local/bin/mevent -c /etc/mevent/server.hdf");
		} else {
			printf("process temproray error %d, %d", ret, try);
		}
	}

	mevent_free(evt);
	return 0;
}
