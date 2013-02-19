/*
 * 事件中心心跳检测重启程序, 需配合 crontab 定时拉起使用
 * 程序本身得错误信息printf，后台的状态输出到日志文件
 * PATH/hb uic 1001 /usr/local/moon/mevent.hdf > /tmp/meventhb.log
 *
 * 正常情况下 输出 后端插件的返回信息
 */
#include <stdio.h>        /* printf() */
#include <unistd.h>        /* malloc(), fork() and getopt() */
#include <stdlib.h>        /* atoi() */
#include <sys/types.h>    /* for pid_t */
#include <string.h>        /* for strcpy() and strlen() */
#include <pthread.h>    /* for pthread_t */

#include "mevent.h"        /* api's mevent.h */

#include "common.h"
#include "net.h"
#include "net-const.h"
#include "log.h"
#include "stats.h"
#include "config.h"

#include "mheads.h"

#include "ClearSilver.h"

int main(int argc, char *argv[])
{
    STRING serror; string_init(&serror);
    int ret;
    char *ename, *conf;
    int cmd;
    int trynum = 5;

    if (argc != 4) {
        printf("Usage: %s EVENT_NAME CMD CONFIG_FILE\n", argv[0]);
        return 1;
    }

    ename = argv[1];
    cmd = atoi(argv[2]);
    conf = argv[3];

    if (config_parse_file(conf, &g_cfg) != 1) {
        printf("parse config file %s failure", conf);
        return 1;
    }
    mtc_init(hdf_get_value(g_cfg, PRE_SERVER".logfile_hb", "/tmp/meventhb"));
    nerr_init();
    merr_init((MeventLog)mtc_msg);

    settings.smsalarm = 0;

    mevent_t *evt = mevent_init_plugin(ename);
    if (evt == NULL) {
        printf("init error\n");
        return 1;
    }

    ret = mevent_trigger(evt, NULL, cmd, FLAGS_SYNC);
    if (PROCESS_OK(ret)) {
        mtc_foo("ok");
        hdf_dump(evt->hdfrcv, NULL);
    } else {
        int tried = 1;
        
    redo:
        string_appendf(&serror, "%d => %d; ", tried, ret);
        sleep(5);
        ret = mevent_trigger(evt, NULL, cmd, FLAGS_SYNC);
        if (PROCESS_NOK(ret) && tried < trynum) {
            tried++;
            goto redo;
        }

        if (PROCESS_NOK(ret) && tried >= trynum) {
            mtc_foo("total  error: %s, restart", serror.buf);
            system("killall -9 mevent; sleep 2; ulimit -S -c 9999999999 && /usr/local/moon/mevent -c /usr/local/moon/server.hdf");
        } else {
            mtc_foo("partly error: %s", serror.buf);
            hdf_dump(evt->hdfrcv, NULL);
        }
    }

    string_clear(&serror);
    mevent_free(evt);
    return 0;
}
