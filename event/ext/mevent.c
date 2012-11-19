/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                         |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                 |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,         |
  | that is bundled with this package in the file LICENSE, and is         |
  | available through the world-wide-web at the following url:             |
  | http://www.php.net/license/3_01.txt                                     |
  | If you did not receive a copy of the PHP license and are unable to     |
  | obtain it through the world-wide-web, please send a note to             |
  | license@php.net so we can mail you a copy immediately.                 |
  +----------------------------------------------------------------------+
  | Author:                                                                 |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.16.2.1.2.1 2007/01/01 19:32:09 iliaa Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_mevent.h"
#include "mevent.h"
#include "ClearSilver.h"
 

typedef enum {
    CNODE_TYPE_STRING = 100,
    CNODE_TYPE_BOOL,
    CNODE_TYPE_INT,
    CNODE_TYPE_INT64,
    CNODE_TYPE_FLOAT,
    CNODE_TYPE_DATETIME,
    CNODE_TYPE_TIMESTAMP,
    CNODE_TYPE_OBJECT,
    CNODE_TYPE_ARRAY,
    
    CNODE_TYPE_POINT = 120,
    CNODE_TYPE_BOX,
    CNODE_TYPE_PATH,
    CNODE_TYPE_TIME,
    CNODE_TYPE_BITOP
} CnodeType;

/* If you declare any globals in php_mevent.h uncomment this:
   ZEND_DECLARE_MODULE_GLOBALS(mevent)
*/

/* True global resources - no need for thread safety here */
static int le_mevent;
//static int le_mevent;

/* {{{ mevent_functions[]
 *
 * Every user visible function must have an entry in mevent_functions[].
 */
zend_function_entry mevent_functions[] = {
    PHP_FE(mevent_init_plugin,    NULL)
    PHP_FE(mevent_free,    NULL)
    PHP_FE(mevent_add_str,    NULL)
    PHP_FE(mevent_add_int,    NULL)
    PHP_FE(mevent_add_bool,    NULL)
    PHP_FE(mevent_add_float,    NULL)
    PHP_FE(mevent_trigger,    NULL)
    PHP_FE(mevent_result,    NULL)
    {NULL, NULL, NULL}    /* Must be the last line in mevent_functions[] */
};
/* }}} */

/* {{{ mevent_module_entry
 */
zend_module_entry mevent_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "mevent",
    mevent_functions,
    PHP_MINIT(mevent),
    PHP_MSHUTDOWN(mevent),
    PHP_RINIT(mevent),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(mevent),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(mevent),
#if ZEND_MODULE_API_NO >= 20010901
    "1.0", /* Replace with version number for your extension */
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MEVENT
ZEND_GET_MODULE(mevent)
#endif


static void php_mevent_dtor(
    zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    mevent_t *mevent_p = (mevent_t*) rsrc->ptr;
    if (mevent_p) {
        mevent_free(mevent_p);
    }
}

static char* mutil_obj_attr(HDF *hdf, char*key)
{
    if (hdf == NULL || key == NULL)
        return NULL;
    
    HDF_ATTR *attr = hdf_obj_attr(hdf);
    while (attr != NULL) {
        if (!strcmp(attr->key, key)) {
            return attr->value;
        }
    }
    return NULL;
}

static void mevent_fetch_array(HDF *node, zval **re)
{
    if (node == NULL) return;

    char *key, *name, *type, *val, *n;
    zval *cre;
    int ctype, namelen;

    node = hdf_obj_child(node);

    while (node) {
        if (hdf_obj_child(node)) {
            key = hdf_obj_name(node) ? hdf_obj_name(node): "unkown";
            ALLOC_INIT_ZVAL(cre);
            array_init(cre);
            mevent_fetch_array(node, &cre);
            add_assoc_zval(*re, key, cre);
        } else if ((val = hdf_obj_value(node)) != NULL) {
            name = hdf_obj_name(node);

            type = mutil_obj_attr(node, "type");
            if (type) ctype = atoi(type);
            else ctype = CNODE_TYPE_STRING;
            
            switch (ctype) {
            case CNODE_TYPE_INT:
            case CNODE_TYPE_BOOL:
            case CNODE_TYPE_INT64:
                add_assoc_long(*re, name, atol(val));
                break;
            case CNODE_TYPE_FLOAT:
                add_assoc_double(*re, name, strtod(val, &n));
                break;
            default:
                namelen = strlen(name);
                add_assoc_string_ex(*re, name, namelen+1, val, 1);
            }
        }

        node = hdf_obj_next(node);
    }
}

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
   PHP_INI_BEGIN()
   STD_PHP_INI_ENTRY("mevent.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_mevent_globals, mevent_globals)
   STD_PHP_INI_ENTRY("mevent.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_mevent_globals, mevent_globals)
   PHP_INI_END()
*/
/* }}} */

/* {{{ php_mevent_init_globals
 */
/* Uncomment this function if you have INI entries
   static void php_mevent_init_globals(zend_mevent_globals *mevent_globals)
   {
   mevent_globals->global_value = 0;
   mevent_globals->global_string = NULL;
   }
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(mevent)
{

    /* If you have INI entries, uncomment these lines 
       REGISTER_INI_ENTRIES();
    */
    le_mevent = zend_register_list_destructors_ex(
        php_mevent_dtor, NULL, PHP_MEVENT_RES_NAME,
        module_number);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(mevent)
{
    /* uncomment this line if you have INI entries
       UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(mevent)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(mevent)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mevent)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "mevent support", "enabled");
    php_info_print_table_row(2, "Version", "2.1");
    php_info_print_table_row(2, "Copyright", "Hunantv.com");
    php_info_print_table_row(2, "author", "neo & bigml");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
       DISPLAY_INI_ENTRIES();
    */
}
/* }}} */


/* {{{ proto resource mevent_init_plugin(string ename)
 */
PHP_FUNCTION(mevent_init_plugin)
{
    int argc = ZEND_NUM_ARGS();
    char *ename = NULL;
    int ename_len;
    long cmd;
    int flags = 0;
     
    mevent_t *mevent_p;

    if (zend_parse_parameters(argc TSRMLS_CC, "s", &ename, &ename_len) == FAILURE) 
        return;

    if (!strcmp(ename, ""))      ename       = "skeleton";

    mevent_p = mevent_init_plugin(ename);

    if (mevent_p == NULL)
        RETURN_LONG(-1);
    
    ZEND_REGISTER_RESOURCE(return_value, mevent_p, le_mevent);
}
/* }}} */


/* {{{ proto int mevent_free(resource db)
 */
PHP_FUNCTION(mevent_free)
{
    int argc = ZEND_NUM_ARGS();
    int db_id = -1;
    zval *db = NULL;
    mevent_t *mevent_p;
    int ret = -1;
    

    if (zend_parse_parameters(argc TSRMLS_CC, "r", &db) == FAILURE) 
        return;

    if (db) {
        ZEND_FETCH_RESOURCE(mevent_p, mevent_t *, &db, db_id,
                            PHP_MEVENT_RES_NAME, le_mevent);
        zend_hash_index_del(&EG(regular_list),
                            Z_RESVAL_P(db));
        //mevent_free(db);
        RETURN_TRUE;
    }
    RETURN_LONG(-1);
}
/* }}} */


/* {{{ proto int mevent_add_str(resource db, string key, string val)
 */
PHP_FUNCTION(mevent_add_str)
{
    char *key = NULL;
    char *val = NULL;
    int argc = ZEND_NUM_ARGS();
    int db_id = -1;
    int key_len;
    int val_len;
    int ret = 0;
    zval *db = NULL;
    mevent_t *mevent_p;
   

    if (zend_parse_parameters(argc TSRMLS_CC, "rss", &db, &key, &key_len,
                              &val, &val_len) == FAILURE) 
        return;

    if (db) {
        ZEND_FETCH_RESOURCE(mevent_p, mevent_t *, &db, db_id,
                            PHP_MEVENT_RES_NAME, le_mevent);
        if (mevent_p) {
            hdf_set_value(mevent_p->hdfsnd, key, val);
            char ztoka[64];
            snprintf(ztoka, 64, "%d", CNODE_TYPE_STRING);
            hdf_set_attr(mevent_p->hdfsnd, key, "type", ztoka);
            RETURN_LONG(0);
        }
    }
}
/* }}} */
 

/* {{{ proto int mevent_add_int(resource db, string key, int val)
 */
PHP_FUNCTION(mevent_add_int)
{
    char *key = NULL;
     
    int argc = ZEND_NUM_ARGS();
    int db_id = -1;
    int key_len;
    int parent_len;
    long val;
    int ret = 0;
    zval *db = NULL;
    mevent_t *mevent_p;
   

    if (zend_parse_parameters(argc TSRMLS_CC, "rsl", &db, &key,
                              &key_len, &val) == FAILURE)
        return;
    
    if (db) {
        ZEND_FETCH_RESOURCE(mevent_p, mevent_t *, &db, db_id,
                            PHP_MEVENT_RES_NAME, le_mevent);
        if (mevent_p) {
            hdf_set_int_value(mevent_p->hdfsnd, key, val);
            char ztoka[64];
            snprintf(ztoka, 64, "%d", CNODE_TYPE_INT);
            hdf_set_attr(mevent_p->hdfsnd, key, "type", ztoka);
            RETURN_LONG(0);
        }
    }
}
/* }}} */
 
/* {{{ proto int mevent_add_bool(resource db, string key, int val)
 */
PHP_FUNCTION(mevent_add_bool)
{
    char *key = NULL;
     
    int argc = ZEND_NUM_ARGS();
    int db_id = -1;
    int key_len;
    int parent_len;
    long val;
    int ret = 0;
    zval *db = NULL;
    mevent_t *mevent_p;
   

    if (zend_parse_parameters(argc TSRMLS_CC, "rsl", &db, &key,
                              &key_len, &val) == FAILURE)
        return;
    
    if (db) {
        ZEND_FETCH_RESOURCE(mevent_p, mevent_t *, &db, db_id,
                            PHP_MEVENT_RES_NAME, le_mevent);
        if (mevent_p) {
            hdf_set_int_value(mevent_p->hdfsnd, key, val);
            char ztoka[64];
            snprintf(ztoka, 64, "%d", CNODE_TYPE_BOOL);
            hdf_set_attr(mevent_p->hdfsnd, key, "type", ztoka);
            RETURN_LONG(0);
        }
    }
}
/* }}} */
 
/* {{{ proto int mevent_add_float(resource db, string key, double val)
 */
PHP_FUNCTION(mevent_add_float)
{
    char *key = NULL;
     
    int argc = ZEND_NUM_ARGS();
    int db_id = -1;
    int key_len;
    int parent_len;
    double val;
    int ret = 0;
    zval *db = NULL;
    mevent_t *mevent_p;
   

    if (zend_parse_parameters(argc TSRMLS_CC, "rsd", &db, &key,
                              &key_len, &val) == FAILURE)
        return;
    
    if (db) {
        ZEND_FETCH_RESOURCE(mevent_p, mevent_t *, &db, db_id,
                            PHP_MEVENT_RES_NAME, le_mevent);
        if (mevent_p) {
            char ztoka[64];
            snprintf(ztoka, 64, "%f", val);
            hdf_set_value(mevent_p->hdfsnd, key, ztoka);
            snprintf(ztoka, 64, "%d", CNODE_TYPE_FLOAT);
            hdf_set_attr(mevent_p->hdfsnd, key, "type", ztoka);
            RETURN_LONG(0);
        }
    }
}
/* }}} */
 
/* {{{ proto int mevent_trigger(resource db, string key, int cmd, int flags)
 */
PHP_FUNCTION(mevent_trigger)
{
    int argc = ZEND_NUM_ARGS();
    char *key; int key_len;
    double cmd, flags = 0;
    int db_id = -1;
    int ret = 0;
     
    zval *db = NULL;
     
    mevent_t *mevent_p;
   

    if (zend_parse_parameters(argc TSRMLS_CC, "rsdd", &db, &key, &key_len, &cmd, &flags) == FAILURE) 
        return;

    if (db) {
        ZEND_FETCH_RESOURCE(mevent_p, mevent_t *, &db, db_id,
                            PHP_MEVENT_RES_NAME, le_mevent);
        if (mevent_p) {
            ret = mevent_trigger(mevent_p, key, cmd, flags);
            RETURN_LONG(ret);
        }
    }
}
/* }}} */

 
/* {{{ proto int mevent_fetch_array(resource db)
 */
PHP_FUNCTION(mevent_result)
{
    int argc = ZEND_NUM_ARGS();
    int db_id = -1;
    zval *db = NULL;
     
    //int i=0;
    mevent_t *mevent_p;

    if (zend_parse_parameters(argc TSRMLS_CC, "r", &db) == FAILURE) 
        return;

    if (db) {
        ZEND_FETCH_RESOURCE(mevent_p, mevent_t *, &db, db_id,
                            PHP_MEVENT_RES_NAME, le_mevent);
        if (mevent_p) {

            array_init(return_value);
             
            if (mevent_p->hdfrcv != NULL) {
                mevent_fetch_array(mevent_p->hdfrcv, &return_value);
            }
        }
    }
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

