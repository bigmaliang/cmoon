#include "mheads.h"

anchor_t* anchor_new(char *name, char *href,
		     char *title, char *target)
{
	anchor_t *anc = (anchor_t*)calloc(1, sizeof(anchor_t));
	if (anc == NULL)
		return NULL;
	
	if (name != NULL)
		strncpy(anc->name, name, sizeof(anc->name)-1);
	if (href != NULL)
		strncpy(anc->href, href, sizeof(anc->href)-1);
	if (title != NULL)
		strncpy(anc->title, title, sizeof(anc->title)-1);
	if (target != NULL)
		strncpy(anc->target, target, sizeof(anc->target)-1);
	
	return anc;
}
void anchor_del(anchor_t *anc)
{
	if (anc != NULL)
		free(anc);
}
