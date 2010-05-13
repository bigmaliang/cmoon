#include "common.h"

/* Define the common structures that are used throughout the whole server. */
struct settings settings;
struct stats stats;
struct mevent *mevent;
HDF *g_cfg;

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
