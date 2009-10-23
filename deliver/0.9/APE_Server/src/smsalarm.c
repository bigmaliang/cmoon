#include <stdarg.h>

#include "main.h"
#include "config.h"
#include "cmd.h"

#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <sys/resource.h>
#include "utils.h"
#include "ticks.h"
#include "log.h"

#include <grp.h>
#include <pwd.h>

#include "adb.h"
#include "extend.h"
#include "hash.h"
#include "smsalarm.h"

void smsalarm_msg(acetables *g_ape, char *msg)
{
	char content[100];

	apeconfig *sec = ape_config_get_section(g_ape->srv, "smsalarm");
	if (sec == NULL) {
		wlog_warn("smsalarm section in ape.conf null");
		return;
	}

	struct _apeconfig_def *def = sec->def;
	while (def != NULL) {
		memset(content, 0x0, sizeof(content));
		snprintf(content, sizeof(content), "%s|%s", def->val, msg);
		
		if (ape_mysql_queryf(g_ape, "smsalarm", "INSERT INTO pei5_smssend (smsContent) "
							 " VALUES ('%s')", content) == NULL) {
			wlog_err("alarm %s error", content);
		}
		
		def = def->next;
	}
}

void smsalarm_msgf(acetables *g_ape, char *fmt, ...)
{
	char msg[1024];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	smsalarm_msg(g_ape, msg);
}
