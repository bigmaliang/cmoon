#include "mevent_plugin.h"

#define PLUGIN_NAME	"skeleton"

struct skeleton_stats {
	size_t memlen;
};

struct skeleton_entry {
	struct event_entry base;
	FILE *fp;
	struct skeleton_stats st;
};

static void skeleton_process_driver(struct event_entry *entry, struct queue_entry *q)
{
	struct skeleton_entry *e = (struct skeleton_entry*)entry;

	struct data_cell *c = data_cell_search(q->dataset, true, DATA_TYPE_STRING, "tmpint");
	if (c != NULL) data_cell_dump(c);

	if (e->fp) usleep(10);

	e->st.memlen = queue_entry_size(q);
	printf("memory len %lu\n", e->st.memlen);
	data_cell_dump(q->dataset);
	
	if ((q->req->flags & FLAGS_SYNC))
		q->req->reply_mini(q->req, REP_OK);
}

static void skeleton_stop_driver(struct event_entry *entry)
{
	struct skeleton_entry *e = (struct skeleton_entry*)entry;

	/*
	 * e->base.name, e->base will free by mevent_stop_driver() 
	 */
	if (e->fp != NULL) fclose(e->fp);
}



static struct event_entry* skeleton_init_driver(void)
{
	struct skeleton_entry *e = calloc(1, sizeof(struct skeleton_entry));
	if (e == NULL) return NULL;

	e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
	e->base.ksize = strlen(PLUGIN_NAME);
	e->base.process_driver = skeleton_process_driver;
	e->base.stop_driver = skeleton_stop_driver;
	
	FILE *fp = fopen("/tmp/skeleton", "w");
	e->fp = fp;

	return (struct event_entry*)e;
}

struct event_driver skeleton_driver = {
	.name = (unsigned char*)PLUGIN_NAME,
	.init_driver = skeleton_init_driver,
};
