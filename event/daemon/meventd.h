#ifndef __FEVENT_H__
#define __FEVENT_H__

#include <sys/types.h>        /* for size_t */
#include <stdint.h>        /* for int64_t */
#include <stdbool.h>
#include "queue.h"

/*
 * private, internal use 
 */
struct mevent {
    size_t numevts;
    size_t hashlen;
    size_t chainlen;
    
    struct event_chain *table;
};

struct event_chain {
    size_t len;
    struct event_entry *first;
    struct event_entry *last;
};

struct timer_entry {
    int timeout;
    bool repeat;
    void (*timer)(struct event_entry *e, unsigned int upsec);
    struct timer_entry *next;
};

struct event_entry {
    /*
     * public, init in mevent_start_driver()
     */
    //void *lib;        /* for dlopen() */
    struct queue *op_queue;
    pthread_t *op_thread;
    int loop_should_stop;
    struct event_entry *prev;
    struct event_entry *next;
    struct timer_entry *timers;

    /*
     * different by plugin, init in init_driver()
     */
    unsigned char *name;
    size_t ksize;
    void (*process_driver)(struct event_entry *e, struct queue_entry *q);
    void (*stop_driver)(struct event_entry *e);

    /*
     * extensions after here...
     */
};
struct event_driver {
    unsigned char *name;
    struct event_entry* (*init_driver)(void);
};

typedef struct event_entry EventEntry;

/*
 * public
 */
struct mevent* mevent_start();
void mevent_stop(struct mevent *evt);
void mevent_add_timer(struct timer_entry **timers, int timeout, bool repeat,
                      void (*timer)(struct event_entry *e, unsigned int upsec));

struct event_entry* find_entry_in_table(struct mevent *evt, const unsigned char *key, size_t ksize);

#endif    /* __FEVENT_H__ */
