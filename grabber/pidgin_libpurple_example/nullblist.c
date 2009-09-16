#include "purple.h"
#include "qq.h"
#include "nullblist.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

static void blist_show(PurpleBuddyList *list)
{
    PurpleBlistNode *node;
    qq_buddy_data *bd;

    node = list ? list->root: NULL;

    while (node != NULL) {
        if (node->type != PURPLE_BLIST_BUDDY_NODE) {
            node = purple_blist_node_next(node, TRUE);
            continue;
        }
        
        bd = (qq_buddy_data*) purple_buddy_get_protocol_data((PurpleBuddy*) node);
        if (bd != NULL)
            printf("list friend %u %s\n", bd->uid, bd->nickname);
        
        node = purple_blist_node_next(node, TRUE);
    }
}

static void blist_newnode(PurpleBlistNode *node)
{
    qq_buddy_data *bd;
    PurpleBuddy *buddy = (PurpleBuddy*)node;

    if (node == NULL || node->type != PURPLE_BLIST_BUDDY_NODE) return;
    
    printf("new friend %s\n", buddy->name);
    
    bd = (qq_buddy_data*) purple_buddy_get_protocol_data(buddy);
    if (bd != NULL && bd->nickname)
        printf("new friend %u %s\n", bd->uid, bd->nickname);
}

static void blist_update(PurpleBuddyList *list, PurpleBlistNode *node)
{
    qq_buddy_data *bd;
    PurpleBuddy *buddy = (PurpleBuddy*)node;
    
    if (node == NULL || node->type != PURPLE_BLIST_BUDDY_NODE) return;

    bd = (qq_buddy_data*) purple_buddy_get_protocol_data(buddy);
    if (bd != NULL && bd->nickname) {
        printf("update friend %u %s\n", bd->uid, bd->nickname);
    }
}

static PurpleBlistUiOps blist_ui_ops =
{
	NULL,
	blist_newnode,
	blist_show,
    blist_update,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

PurpleBlistUiOps* null_blist_get_ui_ops(void)
{
    return &blist_ui_ops;
}
