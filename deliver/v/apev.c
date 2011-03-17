#include "apev.h"

UserEntry* user_new()
{
	return calloc(1, sizeof(UserEntry));
}

SnakeEntry* snake_new(char *name)
{
	if (!name) return NULL;
	
	SnakeEntry *s = calloc(1, sizeof(SnakeEntry));

	s->name = strdup(name);
	s->num_online = 0;
	s->evt = mevent_init_plugin(name);

	if (!s->evt) {
		free(s);
		s = NULL;
	}

	return s;
}
