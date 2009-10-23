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


MYSQL *mysql_instance(acetables *g_ape, char *name)
{
	if (get_property(g_ape->properties, name) != NULL)
		return get_property(g_ape->properties, name)->val;
	return NULL;
}

char *ape_mysql_real_escape_string(const char *str,
								   acetables *g_ape, char *name)
{
	MYSQL *mysql = mysql_instance(g_ape, name);
	unsigned int n = strlen(str);
	char *buf = xmalloc(n*2+1);
        
	mysql_real_escape_string(mysql, buf, str, n);
	
	return buf;
}

MYSQL *ape_mysql_query(const char *query, acetables *g_ape, char *name)
{
	MYSQL *mysql = mysql_instance(g_ape, name);

	if (mysql == NULL) {
		wlog_err("db for %s null", name);
		return NULL;
	}

	if (mysql_query(mysql, query)) {
		wlog_err("[Module] [SQL-WARN] : %s\n", mysql_error(mysql));
		return NULL;
	}
	return mysql;
}

MYSQL *ape_mysql_queryf(acetables *g_ape, char *name, const char *buf, ...)
{
	MYSQL *mysql;
	
	char *buff;

	va_list val;
	
	va_start(val, buf);
	vasprintf(&buff, buf, val);
	va_end(val);
	
	mysql = ape_mysql_query(buff, g_ape, name);
	
	free(buff);
	
	return mysql;
}


MYSQL_RES *ape_mysql_select(const char *query, acetables *g_ape, char *name)
{
	// must be free'd
	MYSQL_RES *res;
	MYSQL *mysql = ape_mysql_query(query, g_ape, name);
	
	if (mysql == NULL) {
		return NULL;
	}
	
	res = mysql_store_result(mysql);
	if (res != NULL && !mysql_num_rows(res)) {
		mysql_free_result(res);
		return NULL;
	}
	return res;
}

MYSQL_ROW ape_mysql_row(const char *query, MYSQL_RES **res,
						acetables *g_ape, char *name)
{
	*res = ape_mysql_select(query, g_ape, name);
	if (*res == NULL) {
		return NULL;
	}
	
	return mysql_fetch_row(*res);
}

MYSQL_RES *ape_mysql_selectf(acetables *g_ape, char *name, char *buf, ...)
{
	MYSQL_RES *res;
	
	char *buff;

	va_list val;
	
	va_start(val, buf);
	vasprintf(&buff, buf, val);
	va_end(val);
	
	res = ape_mysql_select(buff, g_ape, name);
	
	free(buff);
	
	return res;
}


char *ape_mysql_get(const char *query, acetables *g_ape, char *name)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *field;
	
	row = ape_mysql_row(query, &res, g_ape, name);
	if (row != NULL) {
		field = xstrdup(row[0]);
		mysql_free_result(res);
		
		return field;
	}
	return NULL;
}

