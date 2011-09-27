#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define MAX_PHRASE_LEN 15
#define MAX_PHRASE_S_LEN 45

#define BITMAP_SET(bm, pos)     (*(bm+pos/8) |= 0x01 << (pos%8))
#define BITMAP_GET(bm, pos)     (*(bm+(pos/8)) & (0x01 << (pos%8)))
/*
 * caculate word's bitmap positon
 * currently we used a (0xFFFFFFFF / 8) bytes memory bitmap
 * word's location is caculated by following:
 * yyyy xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx
 *   I, the highest four bit yyyy indicate word's total
 *      chinese character length except non-utf8 char
 *      so, the max_phrase_len is 0xF, 15
 *      e.g. yyyy == 0x10 when word = 天气
 *           yyyy == 0x1  when word = hold姐
 *  II, the remain 28 bit is assembled by string, detail logic, check the code please
 *  Currently, the rate of accuracy postion is 99% (215366/217416).
 *  if you have any improvemention, please let bigmaliang@gmail.com know
 *  And, as a remind, you need to check UTF-8 's spec to read the BITMAP_POS logic
 */
#define BITMAP_POS(s, len, pos)                                     \
    if (1) {                                                        \
        int _b, _c;                                                 \
        char *_p;                                                   \
        unsigned int _sum;                                          \
        int _tc;                                                    \
        _p = s;                                                     \
        _c = 0;                                                     \
        while (*_p && _p - s < len) {                               \
            if ((*_p & 0xE0) != 0xE0 ||                             \
                (*(_p+1) & 0x80) != 0x80 ||                         \
                (*(_p+2) & 0x80) != 0x80) {                         \
                _c++;                                               \
                _p++;                                               \
            } else {                                                \
                _p = _p + 3;                                        \
            }                                                       \
        }                                                           \
        _tc = (len - _c) / 3;                                       \
                                                                    \
        pos = 0;                                                    \
        _p = s;                                                     \
        _sum = 0;                                                   \
        _b = 3;                                                     \
        _c = 0;                                                     \
        while (*_p && _p - s < len) {                               \
            if (_b == 3 &&                                          \
                ( (*_p & 0xE0) != 0xE0 ||                           \
                  (*(_p+1) & 0x80) != 0x80 ||                       \
                  (*(_p+2) & 0x80) != 0x80) ) {                     \
                pos = pos + *_p;                                    \
                _p++;                                               \
                continue;                                           \
            }                                                       \
            _b--;                                                   \
            if (_b == 2)                                            \
                _sum = (*_p&0x0F) << 12 ;                           \
            else                                                    \
                _sum = _sum | ( (*_p&0x3F) << (_b*6) );             \
            if (_b == 0) {                                          \
                _c++;                                               \
                switch (_tc) {                                      \
                case 2:                                             \
                    if (pos == 0)                                   \
                        pos = pos | (_sum & 0xFFFF) << 12;          \
                    else {                                          \
                        pos = pos | (_sum & 0xFFF);                 \
                    }                                               \
                    break;                                          \
                case 1:                                             \
                    pos = pos + _sum;                               \
                    break;                                          \
                case 3:                                             \
                    pos = pos | ((_sum & 0x1FF) << ((_c-1)*9));     \
                    break;                                          \
                case 4:                                             \
                    pos = pos | ((_sum & 0x7F) << ((_c-1)*7));      \
                    break;                                          \
                case 5:                                             \
                    if (_c == 5)                                    \
                        pos = pos | ((_sum & 0xFF) << 20);          \
                    else                                            \
                        pos = pos | ((_sum & 0x1F) << ((_c-1)*5));  \
                    break;                                          \
                case 6:                                             \
                    if (_c == 6)                                    \
                        pos = pos | ((_sum & 0xFF) << 20);          \
                    else                                            \
                        pos = pos | ((_sum & 0xF) << ((_c-1)*4));   \
                    break;                                          \
                case 7:                                             \
                    pos = pos | ((_sum & 0xF) << ((_c-1)*4));       \
                    break;                                          \
                case 8:                                             \
                    if (_c % 2 == 1)                                \
                        pos = pos | ((_sum & 0x7F) << ((_c-1)*7));  \
                    break;                                          \
                default:                                            \
                    if (_c % 2 == 1 && _c < 15)                     \
                        pos = pos | ((_sum & 0xF) << ((_c-1)*4));   \
                    break;                                          \
                }                                                   \
                pos = (pos & 0xFFFFFFF) | ((_tc & 0xF) << 28);      \
                                                                    \
                _sum = 0;                                           \
                _b = 3;                                             \
            }                                                       \
            _p++;                                                   \
        }                                                           \
    }
    

/*
 * set an chinese word into bitmap
 * normarlly called mannny times on process initialize
 * parameters:
 * bm   :bitmap which ts set into
 * ts   :an chinese word, e.g. 天气
 */
void word_set(char *bm, char *ts)
{
    unsigned int pos = 0;
    
    if (!ts || strlen(ts) > MAX_PHRASE_S_LEN) return;

    BITMAP_POS(ts, strlen(ts), pos);
    //char *s = ts; int len = strlen(ts);
    
    if (BITMAP_GET(bm, pos)) {
        //printf("%x seted\n", pos);
    }

    //if (pos == 0x24e1373a) printf("%s seted\n", ts);

    BITMAP_SET(bm, pos);
}

/*
 * divide ts into chinese, english, or, chiglish words
 * bm   :bitmap initialized on word_set()
 * ts   :input string e.g. 早上天气不错，确被hold姐雷翻了。
 * len  :input string size
 * cbk  :the function called on words founded
 */
void word_split(char *bm, char *ts, size_t len, void (*cbk)(char *s, size_t len))
{
    char *s, *t, *e, *es;
    unsigned int pos;
    int r;
    int i;
    
    if (!bm || !ts || !cbk || len <= 0) return;

    s = ts;

    while (*s) {
        es = NULL;
        t = s;
        r = len - (s - ts);

        for (i = 0; i < MAX_PHRASE_S_LEN && r >= 0; ) {
            e = s + i;
            BITMAP_POS(s, i, pos);
            if (BITMAP_GET(bm, pos)) {
                t = s + i;
                cbk(s, e - s);
            }
            
            while (*e && e - ts < len) {
                if ((*e & 0xE0) != 0xE0 ||
                    (*(e+1) & 0x80) != 0x80 ||
                    (*(e+2) & 0x80) != 0x80) {
                    if (!es) es = e;
                    e++;
                } else break;
            }
            if (e != s + i) {
                if (i == 0) {
                    t = e;
                    cbk(es, e - es);
                }
                i = i + (e - es);
                r = r - (e - es);
            } else {
                i = i + 3;
                r = r - 3;
            }
        }

        if (t > s) s = t;
        else s = s + 3;
    }
}

/*
 * output aux
 */
static char words[100][MAX_PHRASE_S_LEN];
static int wpos;
static void on_words(char *s, size_t len)
{
    if (!s) return;
    
    memcpy(words[wpos], s, len);
    words[wpos++][len] = '\0';
}
static void out_words()
{
    int i;
    for (i = 0; i < wpos; i++) {
        printf("%s\n", words[i]);
    }
}

int main(int argc, char **argv, char **envp)
{

    char ts[1000];
    FILE *fp;
    size_t len;
    int cnt;

    char *bm = calloc(1, 0xFFFFFFFF / 0x8); /* about 512MB */

#if 1
    cnt = 0;
    if ((fp = fopen("dict.txt", "r"))) {
        while (fgets(ts, 1000, fp)) {
            len = strlen(ts)-2;
            ts[len] = '\0';
            if (len <= MAX_PHRASE_S_LEN) {
                cnt++;
                word_set(bm, ts);
            }
        }
        printf("%d words seted\n", cnt);
    } else {
        printf("open dict.txt failed\n");
        return 1;
    }
#endif

#if 0
    word_set(bm, "专场");
    word_set(bm, "专机");
#endif

    if (argc > 1) {
        memset(words, sizeof(words), 0x0);
        wpos = 0;

        struct timeval tv_s, tv_e;
        unsigned long usec;
        
        gettimeofday(&tv_s, NULL);
        word_split(bm, argv[1], strlen(argv[1]), on_words);
        gettimeofday(&tv_e, NULL);
        usec = (tv_e.tv_sec - tv_s.tv_sec) * 1000000ul + (tv_e.tv_usec - tv_s.tv_usec);

        printf("parse finished in %.6f seconds\n", (double)(usec / 1000000.0));
        printf("words:\n");
        out_words();
    } else {
        unsigned int i;
        cnt = 0;
        for (i = 0; i < 0xFFFFFFFF; i++) {
            if (BITMAP_GET(bm, i)) cnt++;
        }
        
        printf("%d distinct word\n", cnt);
    }

    free(bm);

    return 0;
}
