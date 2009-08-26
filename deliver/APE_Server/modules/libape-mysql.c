#include "plugins.h"
#include "global_plugins.h"

#include <stdio.h>

#define MODULE_NAME "mysql"


int ape_mysql_connect(acetables *g_ape);

static ace_plugin_infos infos_module = {
	"MySQL support",
	"0.01",
	"Anthony Catel",
	"mod_mysql.conf"
};

static void init_module(acetables *g_ape)
{
	MYSQL *mysql = mysql_init(NULL);
	MYSQL *smsdb = mysql_init(NULL);
	
	my_bool reconnect = 1;
	mysql_options(mysql, MYSQL_OPT_RECONNECT, &reconnect);
	mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");
	mysql_options(mysql, MYSQL_INIT_COMMAND, "SET NAMES 'utf8'");
	
	mysql_options(smsdb, MYSQL_OPT_RECONNECT, &reconnect);
	mysql_options(smsdb, MYSQL_SET_CHARSET_NAME, "utf8");
	mysql_options(smsdb, MYSQL_INIT_COMMAND, "SET NAMES 'utf8'");

	add_property(&g_ape->properties, "userinfo", mysql, EXTEND_POINTER, EXTEND_ISPRIVATE);
	add_property(&g_ape->properties, "smsalarm", smsdb, EXTEND_POINTER, EXTEND_ISPRIVATE);
	
	/* init the connection to MySQL */
	if (!ape_mysql_connect(g_ape)) {
		fprintf(stderr, "[Module] [ERR] Connexion SQL : %s, %s\n",
				mysql_error(mysql), mysql_error(smsdb));
		exit(0);
	}
}

int ape_mysql_connect(acetables *g_ape)
{
	MYSQL *mysql = mysql_instance(g_ape, "userinfo");
	
	if (!mysql_real_connect(mysql, READ_CONF("db_server"), READ_CONF("db_user"),
							READ_CONF("db_password"), READ_CONF("db_name"),
							0, NULL, CLIENT_FOUND_ROWS)) {
		return 0;
	}
	mysql->reconnect = 1;

	mysql = mysql_instance(g_ape, "smsalarm");
	if (!mysql_real_connect(mysql, READ_CONF("smsa_server"), READ_CONF("smsa_user"),
							READ_CONF("smsa_password"), READ_CONF("smsa_name"),
							0, NULL, CLIENT_FOUND_ROWS)) {
		return 0;
	}
	mysql->reconnect = 1;
	return 1;
}

static ace_callbacks callbacks = {
	NULL,				/* Called when new user is added */
	NULL,				/* Called when a user is disconnected */
	NULL,				/* Called when new chan is created */
	NULL,				/* Called when a user join a channel */
	NULL				/* Called when a user leave a channel */
};

APE_INIT_PLUGIN(MODULE_NAME, init_module, callbacks)

