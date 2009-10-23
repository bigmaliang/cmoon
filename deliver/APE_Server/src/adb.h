#ifndef __ADB_H__
#define __ADB_H__

#include <mysql/mysql.h>
#include "main.h"

MYSQL *mysql_instance(acetables *g_ape, char *name);
MYSQL *ape_mysql_query(const char *query, acetables *g_ape, char *name);
MYSQL *ape_mysql_queryf(acetables *g_ape, char *name, const char *buf, ...);
MYSQL_RES *ape_mysql_select(const char *query, acetables *g_ape, char *name);
MYSQL_RES *ape_mysql_selectf(acetables *g_ape, char *name, char *buf, ...);
MYSQL_ROW ape_mysql_row(const char *query, MYSQL_RES **res, acetables *g_ape, char *name);
char *ape_mysql_get(const char *query, acetables *g_ape, char *name);
char *ape_mysql_real_escape_string(const char *str, acetables *g_ape, char *name);

#endif	/* __ADB_H__ */
