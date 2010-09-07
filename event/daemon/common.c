#include <stdbool.h>
#include <event.h>

#include "common.h"

/* Define the common structures that are used throughout the whole server. */
struct settings settings;
struct stats stats;
struct mevent *mevent;
HDF *g_cfg;
volatile time_t g_ctime;


static void set_current_time(void)
{
    g_ctime = time(NULL);
}

void clock_handler(const int fd, const short which, void *arg)
{
    struct timeval t = {.tv_sec = 1, .tv_usec = 0};
    static bool initialized = false;
	struct event *clock_evt = (struct event*) arg;

    if (initialized) {
        /* only delete the event if it's actually there. */
        evtimer_del(clock_evt);
    } else {
        initialized = true;
    }

    evtimer_set(clock_evt, clock_handler, clock_evt);
    evtimer_add(clock_evt, &t);

    set_current_time();
}

// Explode a string in an array.
size_t explode(const char split, char *input, char **tP, unsigned int limit)
{
	size_t i = 0;
	
	tP[0] = input;
	for (i = 0; *input; input++) {
		if (*input == split) {
			i++;
			*input = '\0';
			if(*(input + 1) != '\0' && *(input + 1) != split) {
				tP[i] = input + 1;
			} else {
				i--;
			}
		}
		if ((i+1) == limit) {
			return i;
		}
	}
	
	return i;
}

