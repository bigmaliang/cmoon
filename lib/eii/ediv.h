#ifndef __EDIV_H__
#define __EDIV_H__

#include "eheads.h"

__BEGIN_DECLS

#define MAX_PHRASE_LEN 15
#define MAX_PHRASE_S_LEN 45

/*
 * apply div's bitmap memory, about 512M, so, check the return please
 */
NEOERR* ediv_init(char **bm);

/*
 * apply memory if need, and set words from file
 */
NEOERR* ediv_init_from_file(char **bm, char *path, int *wnum);

/*
 * cleanup ediv's memory, I'm a monster, REMEMBER TO DESTROY ME
 */
void ediv_destroy(char **bm);


/*
 * set an chinese word into bitmap
 * normarlly called mannny times on process initialize
 * parameters:
 * bm   :bitmap which ts set into
 * w    :an word, e.g. 天气 hold姐
 */
void ediv_word_set(char *bm, char *w);

/*
 * return the distinct word number
 */
unsigned int ediv_word_distinct(char *bm);

enum {
    EDIV_SOPT_NONE = 0,
    EDIV_SOPT_ONLY_MAXMATCH = (1 << 0), /* max matches dictionary's word */
    EDIV_SOPT_THROW_UNKNOWN = (1 << 1), /* throw away chars not in dictionary */
    EDIV_SOPT_SKIP_NUTF = (1 << 2)      /* don't split non utf8 chinese characters */
};

/*
 * divide s into chinese, english, or, chiglish words
 * bm   :bitmap initialized on word_set()
 * ts   :input string e.g. 早上天气不错，却被hold姐雷翻了。
 * len  :input string size
 * wnum :word number founded
 * cbk  :the function called on words founded
 * opt  :split option
 */
NEOERR* ediv_word_split(char *bm, char *ts, size_t len, int *wnum,
                        void(*cbk)(char *w, size_t len), int opt);

__END_DECLS
#endif    /* __EDIV_H__ */
