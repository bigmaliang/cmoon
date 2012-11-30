#ifndef __MTPL_H__
#define __MTPL_H__

#include "mheads.h"

/*
 * mtemplate, the template syntax of moon, powered by clearsilver
 */

__BEGIN_DECLS

NEOERR* mtpl_set_tplpath(char *dir);
NEOERR* mtpl_append_loadpath(char *dir);

/*
 * in Config Rend
 * we need to build some complex data in hdf format(e.g. email data),
 * the finally data usually made up by the following 3 parts:
 *   In    : value from runtime variable. e.g. send to
 *   Config: value from config file. e.g. send option
 *   Rend  : value from template file. e.g. email content
 * so, I offered InConfigRend_xxx function
 */

/*
 * init: parse a directory, find *.hdf config file,
 * parse and save them into datah(with 3 member) with key parameter:
 *  valueo(hdf)   store Valueset children by NodeName
 *  layouto(hdf)  store Layout children   by NodeName
 *  layouth(hash) store Layout children's csparser by NodeName_ChildNodeName
 */
NEOERR* mtpl_InConfigRend_init(char *dir, char *key, HASH **datah);

/*
 * build the out hdf, through in hdf, and key+datah
 * hdf in could made by the following 3 parts:
 * PRE_DATASET  :data used by template file on cs_rend()
 * PRE_VALUESET :copy to out direct
 * PRE_VALUEREP :replace valueset after valueset copied
 */
NEOERR* mtpl_InConfigRend_get(HDF *out, HDF *in, char *key, char *name, HASH *datah);
void mtpl_InConfigRend_destroy(HASH *datah);

__END_DECLS
#endif    /* __MTPL_H__ */
