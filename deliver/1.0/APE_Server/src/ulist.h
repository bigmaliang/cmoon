/*
 * Neotonic ClearSilver Templating System
 *
 * This code is made available under the terms of the 
 * Neotonic ClearSilver License.
 * http://www.neotonic.com/clearsilver/license.hdf
 *
 * Copyright (C) 2001 by Brandon Long
 */

#ifndef __ULIST_H_
#define __ULIST_H_ 1

typedef struct _ulist
{
  int flags;
  void **items;
  int num;
  int max;
} ULIST;

#define ULIST_INTEGER (1<<0)
#define ULIST_FREE (1<<1)
#define ULIST_COPY (1<<2)

#define STATUS_OK			0
#define STATUS_FAILURE		1
#define STATUS_NOMEN		2
#define STATUS_OUTOFRANGE	3

int uListInit(ULIST **ul, int size, int flags);
int uListvInit(ULIST **ul, ...);
int uListLength (ULIST *ul);
int uListAppend (ULIST *ul, void *data);
int uListPop (ULIST *ul, void **data);
int uListInsert (ULIST *ul, int x, void *data);
int uListDelete (ULIST *ul, int x, void **data);
int uListGet (ULIST *ul, int x, void **data);
int uListSet (ULIST *ul, int x, void *data);
int uListReverse (ULIST *ul);
int uListSort (ULIST *ul, int (*compareFunc)(const void*, const void*));
void *uListSearch (ULIST *ul, const void *key, int (*compareFunc)(const void *, const void*));
void *uListIn (ULIST *ul, const void *key, int (*compareFunc)(const void *, const void*));
int uListIndex (ULIST *ul, const void *key, int (*compareFunc)(const void *, const void*));
int uListDestroy (ULIST **ul, int flags);
int uListDestroyFunc (ULIST **ul, void (*destroyFunc)(void *));

#endif /* __ULIST_H_ */
