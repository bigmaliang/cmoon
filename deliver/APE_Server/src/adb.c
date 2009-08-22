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


MYSQL *mysql_instance(acetables *g_ape)
{
	return get_property(g_ape->properties, "mysql")->val;
}

char *ape_mysql_real_escape_string(const char *str, acetables *g_ape)
{
        MYSQL *mysql = mysql_instance(g_ape);
	unsigned int n = strlen(str);
        char *buf = xmalloc(n*2+1);
        
	mysql_real_escape_string(mysql, buf, str, n);
	
	return buf;
}

MYSQL *ape_mysql_query(const char *query, acetables *g_ape)
{
	MYSQL *mysql = mysql_instance(g_ape);

	if (mysql_query(mysql, query)) {
		fprintf(stderr, "[Module] [SQL-WARN] : %s\n", mysql_error(mysql));
		return NULL;
	}
	return mysql;
}

MYSQL *ape_mysql_queryf(acetables *g_ape, const char *buf, ...)
{
	MYSQL *mysql;
	
	char *buff;

	va_list val;
	
	va_start(val, buf);
	vasprintf(&buff, buf, val);
	va_end(val);
	
	mysql = ape_mysql_query(buff, g_ape);
	
	free(buff);
	
	return mysql;
}


MYSQL_RES *ape_mysql_select(const char *query, acetables *g_ape)
{
	// must be free'd
	MYSQL_RES *res;
	MYSQL *mysql = ape_mysql_query(query, g_ape);
	
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

MYSQL_ROW ape_mysql_row(const char *query, MYSQL_RES **res, acetables *g_ape)
{
	*res = ape_mysql_select(query, g_ape);
	if (*res == NULL) {
		return NULL;
	}
	
	return mysql_fetch_row(*res);
}

MYSQL_RES *ape_mysql_selectf(acetables *g_ape, char *buf, ...)
{
	MYSQL_RES *res;
	
	char *buff;

	va_list val;
	
	va_start(val, buf);
	vasprintf(&buff, buf, val);
	va_end(val);
	
	res = ape_mysql_select(buff, g_ape);
	
	free(buff);
	
	return res;
}


char *ape_mysql_get(const char *query, acetables *g_ape)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	char *field;
	
	row = ape_mysql_row(query, &res, g_ape);
	if (row != NULL) {
		field = xstrdup(row[0]);
		mysql_free_result(res);
		
		return field;
	}
	return NULL;
}

