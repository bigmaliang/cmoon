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
    s->idle = time(NULL);
    s->state = RUNNING;
    
    s->num_online = 0;
    s->evt = mevent_init_plugin(name);

    if (!s->evt) {
        free(s);
        s = NULL;
    }

    return s;
}

ChanEntry* channel_new(char *cname)
{
    if (!cname) return NULL;

    ChanEntry *c = calloc(1, sizeof(ChanEntry));
    c->name = strdup(cname);
    c->x_missed = NULL;

    return c;
}

NameEntry* name_new(char *name)
{
    if (!name) return NULL;

    NameEntry *n = calloc(1, sizeof(NameEntry));
    n->name = strdup(name);
    n->next = NULL;

    return n;
}

NameEntry* name_push(char *name, NameEntry **entry)
{
    if (name_find(*entry, name) >= 0) return *entry;

    NameEntry *n = name_new(name);
    n->next = *entry;
    *entry = n;

    return n;
}

NameEntry* name_find(NameEntry *entry, char *name)
{
    while (entry) {
        if (strcmp(name, entry->name) == 0)
            return entry;

        entry = entry->next;
    }

    return NULL;
}

NameEntry* name_remove(char *name, NameEntry **entry)
{
    if (!*entry || !name) return NULL;

    NameEntry *e = *entry, *b = *entry;

    while (e && e->name) {
        if (strcmp(name, e->name) == 0) {
            if (b == e) *entry = NULL;
            else b->next = e->next;

            break;
        }

        b = e;
        e = e->next;
    }

    name_free(e);

    return *entry;
}

void name_free(NameEntry *e)
{
    if (!e) return;

    if (e->name) free(e->name);

    free(e);
}
