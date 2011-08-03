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
#include "php_nevent.h"
#include "mevent.h"
#include "ClearSilver.h"
 

/* If you declare any globals in php_nevent.h uncomment this:
   ZEND_DECLARE_MODULE_GLOBALS(nevent)
*/

/* True global resources - no need for thread safety here */
static int le_nevent;
//static int le_nevent;

/* {{{ nevent_functions[]
 *
 * Every user visible function must have an entry in nevent_functions[].
 */
zend_function_entry nevent_functions[] = {
    PHP_FE(nevent_init_plugin,    NULL)
    PHP_FE(nevent_free,    NULL)
    PHP_FE(nevent_add_str,    NULL)
    PHP_FE(nevent_add_u32,    NULL)
    PHP_FE(nevent_trigger,    NULL)
    PHP_FE(nevent_result,    NULL)
    {NULL, NULL, NULL}    /* Must be the last line in nevent_functions[] */
};
/* }}} */

/* {{{ nevent_module_entry
 */
zend_module_entry nevent_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "nevent",
    nevent_functions,
    PHP_MINIT(nevent),
    PHP_MSHUTDOWN(nevent),
    PHP_RINIT(nevent),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(nevent),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(nevent),
#if ZEND_MODULE_API_NO >= 20010901
    "1.0", /* Replace with version number for your extension */
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_NEVENT
ZEND_GET_MODULE(nevent)
#endif


static void php_nevent_dtor(
    zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    mevent_t *evt = (mevent_t*) rsrc->ptr;
    if (evt) {
        mevent_free(evt);
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

static void nevent_fetch_array(HDF *node, zval **re)
{
    if (node == NULL) return;

    char *key, *type, *val;
    zval *cre;

    node = hdf_obj_child(node);

    while (node) {
        if ((val = hdf_obj_value(node)) != NULL) {
            type = mutil_obj_attr(node, "type");
            if (type && !strcmp(type, "int")) {
                add_assoc_long(*re, hdf_obj_name(node), atoi(val));
            } else {
                add_assoc_string_ex(*re, hdf_obj_name(node),
                                    (strlen(hdf_obj_name(node))+1),
                                    val, 1);
            }
        }

        if (hdf_obj_child(node)) {
            key = hdf_obj_name(node) ? hdf_obj_name(node): "unkown";
            ALLOC_INIT_ZVAL(cre);
            array_init(cre);
            nevent_fetch_array(node, &cre);
            add_assoc_zval(*re, key, cre);
        }

        node = hdf_obj_next(node);
    }
}

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
   PHP_INI_BEGIN()
   STD_PHP_INI_ENTRY("nevent.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_nevent_globals, nevent_globals)
   STD_PHP_INI_ENTRY("nevent.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_nevent_globals, nevent_globals)
   PHP_INI_END()
*/
/* }}} */

/* {{{ php_nevent_init_globals
 */
/* Uncomment this function if you have INI entries
   static void php_nevent_init_globals(zend_nevent_globals *nevent_globals)
   {
   nevent_globals->global_value = 0;
   nevent_globals->global_string = NULL;
   }
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(nevent)
{

    /* If you have INI entries, uncomment these lines 
       REGISTER_INI_ENTRIES();
    */
    le_nevent = zend_register_list_destructors_ex(
        php_nevent_dtor, NULL, PHP_NEVENT_RES_NAME,
        module_number);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(nevent)
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
PHP_RINIT_FUNCTION(nevent)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(nevent)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(nevent)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "nevent support", "enabled");
    php_info_print_table_row(2, "Version", "2.0");
    php_info_print_table_row(2, "Copyright", "Hunantv.com");
    php_info_print_table_row(2, "author", "neo & bigml");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
       DISPLAY_INI_ENTRIES();
    */
}
/* }}} */


/* {{{ proto resource nevent_init_plugin(string ename)
 */
PHP_FUNCTION(nevent_init_plugin)
{
    int argc = ZEND_NUM_ARGS();
    char *ename = NULL;
    int ename_len;
    long cmd;
    int flags = 0;
     
    mevent_t *evt;

    if (zend_parse_parameters(argc TSRMLS_CC, "s", &ename, &ename_len) == FAILURE) 
        return;

    if (!strcmp(ename, ""))      ename       = "skeleton2";

    evt = mevent_init_plugin(ename);

    if (evt == NULL)
        RETURN_LONG(-1);
    
    ZEND_REGISTER_RESOURCE(return_value, evt, le_nevent);
}
/* }}} */


/* {{{ proto int nevent_free(resource db)
 */
PHP_FUNCTION(nevent_free)
{
    int argc = ZEND_NUM_ARGS();
    int db_id = -1;
    zval *db = NULL;
    mevent_t *evt;
    int ret = -1;
    

    if (zend_parse_parameters(argc TSRMLS_CC, "r", &db) == FAILURE) 
        return;

    if (db) {
        ZEND_FETCH_RESOURCE(evt, mevent_t *, &db, db_id,
                            PHP_NEVENT_RES_NAME, le_nevent);
        zend_hash_index_del(&EG(regular_list),
                            Z_RESVAL_P(db));
        //nevent_free(db);
        RETURN_TRUE;
    }
    RETURN_LONG(-1);
}
/* }}} */


/* {{{ proto int nevent_add_str(resource db, string key, string val)
 */
PHP_FUNCTION(nevent_add_str)
{
    char *key = NULL;
    char *val = NULL;
    int argc = ZEND_NUM_ARGS();
    int db_id = -1;
    int key_len;
    int val_len;
    int ret = 0;
    zval *db = NULL;
    mevent_t *evt;
   

    if (zend_parse_parameters(argc TSRMLS_CC, "rss", &db, &key, &key_len,
                              &val, &val_len) == FAILURE) 
        return;

    if (db) {
        ZEND_FETCH_RESOURCE(evt, mevent_t *, &db, db_id,
                            PHP_NEVENT_RES_NAME, le_nevent);
        if (evt) {
            hdf_set_value(evt->hdfsnd, key, val);
            RETURN_LONG(0);
        }
    }
}
/* }}} */
 

/* {{{ proto int nevent_add_u32(resource db, string key, int val)
 */
PHP_FUNCTION(nevent_add_u32)
{
    char *key = NULL;
     
    int argc = ZEND_NUM_ARGS();
    int db_id = -1;
    int key_len;
    int parent_len;
    long val;
    int ret = 0;
    zval *db = NULL;
    mevent_t *evt;
   

    if (zend_parse_parameters(argc TSRMLS_CC, "rsl", &db, &key,
                              &key_len, &val) == FAILURE)
        return;
    
    if (db) {
        ZEND_FETCH_RESOURCE(evt, mevent_t *, &db, db_id,
                            PHP_NEVENT_RES_NAME, le_nevent);
        if (evt) {
            hdf_set_int_value(evt->hdfsnd, key, val);
            RETURN_LONG(0);
        }
    }
}
/* }}} */
 
/* {{{ proto int nevent_trigger(resource db, string key, int cmd, int flags)
 */
PHP_FUNCTION(nevent_trigger)
{
    int argc = ZEND_NUM_ARGS();
    char *key; int key_len;
    double cmd, flags = 0;
    int db_id = -1;
    int ret = 0;
     
    zval *db = NULL;
     
    mevent_t *evt;
   

    if (zend_parse_parameters(argc TSRMLS_CC, "rsdd", &db, &key, &key_len, &cmd, &flags) == FAILURE) 
        return;

    if (db) {
        ZEND_FETCH_RESOURCE(evt, mevent_t *, &db, db_id,
                            PHP_NEVENT_RES_NAME, le_nevent);
        if (evt) {
            ret = mevent_trigger(evt, key, cmd, flags);
            RETURN_LONG(ret);
        }
    }
}
/* }}} */

 
/* {{{ proto int nevent_fetch_array(resource db)
 */
PHP_FUNCTION(nevent_result)
{
    int argc = ZEND_NUM_ARGS();
    int db_id = -1;
    zval *db = NULL;
     
    //int i=0;
    mevent_t *evt;

    if (zend_parse_parameters(argc TSRMLS_CC, "r", &db) == FAILURE) 
        return;

    if (db) {
        ZEND_FETCH_RESOURCE(evt, mevent_t *, &db, db_id,
                            PHP_NEVENT_RES_NAME, le_nevent);
        if (evt) {

            array_init(return_value);
             
            if (evt->hdfrcv != NULL) {
                nevent_fetch_array(evt->hdfrcv, &return_value);
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

