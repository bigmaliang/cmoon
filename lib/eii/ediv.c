#include "eheads.h"

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
                pos = ((pos << 5) + 5381) + *_p;                    \
                pos = (pos & 0xFFFFFFF) | ((_tc & 0xF) << 28);      \
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

NEOERR* ediv_init(char **bm)
{
    if (!bm) return nerr_raise(NERR_ASSERT, "paramter null");

    *bm = calloc(1, 0xFFFFFFFF / 0x8);
    if (!(*bm)) return nerr_raise(NERR_NOMEM, "bitmap init");

    return STATUS_OK;
}

NEOERR* ediv_init_from_file(char **bm, char *path, int *wnum)
{
    FILE *fp;
    char line[MAX_PHRASE_S_LEN];
    int len;
    NEOERR *err;
    
    if (!bm || !path) return nerr_raise(NERR_ASSERT, "paramter null");

    if (*bm == NULL) {
        err = ediv_init(bm);
        if (err != STATUS_OK) return nerr_pass(err);
    }

    fp = fopen(path, "r");
    if (fp) {
        while (fgets(line, MAX_PHRASE_S_LEN, fp)) {
            len = strlen(line) - 1;
            while (len > 0 && line[len] == '\n') line[len--] = '\0';
            while (len > 0 && line[len] == '\r') line[len--] = '\0';
            ediv_word_set(*bm, line);
            if (wnum) (*wnum)++;
        }
        fclose(fp);
    } else return nerr_raise(NERR_IO, "Unable to open %s for reading", path);

    return STATUS_OK;
}

void ediv_destroy(char **bm)
{
    if (!bm) return;
    free(*bm);
    *bm = NULL;
}

void ediv_word_set(char *bm, char *w)
{
    unsigned int pos = 0;
    size_t len;

    if (!w) return;

    len = strlen(w);
    
    if (len > MAX_PHRASE_S_LEN) return;

    BITMAP_POS(w, len, pos);
    
    BITMAP_SET(bm, pos);
}

unsigned int ediv_word_distinct(char *bm)
{
    unsigned int cnt = 0;
    
    for (unsigned int i = 0; i < 0xFFFFFFFF; i++) {
        if (BITMAP_GET(bm, i)) cnt++;
    }
    
    return cnt;
}


NEOERR* ediv_word_split(char *bm, char *ts, size_t len, int *wnum,
                        void(*cbk)(char *w, size_t len), int opt)
{
    char *s, *t, *e, *es;
    unsigned int pos;
    int r;
    int i;
    
    if (!bm || !ts || len <= 0) return nerr_raise(NERR_ASSERT, "paramter null");

    s = ts;

    while (s - ts < len && *s) {
        es = NULL;
        t = s;
        r = len - (s - ts);

        for (i = 0; i < MAX_PHRASE_S_LEN && r >= 0; ) {
            e = s + i;
            BITMAP_POS(s, i, pos);
            if (BITMAP_GET(bm, pos)) {
                t = s + i;
                if (!(opt & EDIV_SOPT_ONLY_MAXMATCH) && cbk) cbk(s, e - s);
                if (wnum) (*wnum)++;
                if (opt & EDIV_SOPT_ONLY_MINMATCH); break;
            }

            if (opt & EDIV_SOPT_SKIP_NUTF) {
                /*
                 * skip non-utf8 chinese char
                 */
                while (e - ts < len && *e) {
                    if ((*e & 0xE0) != 0xE0 ||
                        (*(e+1) & 0x80) != 0x80 ||
                        (*(e+2) & 0x80) != 0x80) {
                        if (!es) es = e;
                        e++;
                    } else break;
                }
                if (e != s + i) {
                    /* "ES...E" are non-utf8 chinese characters */
                    if (i == 0) {
                        t = e;
                        /* prevent duplicate cbk on engStart */
                        if (!(opt & EDIV_SOPT_ONLY_MAXMATCH) && cbk && es) {
                            cbk(es, e - es);
                            es = NULL;
                        }
                        if (wnum) (*wnum)++;
                    }
                    i = i + (e - es);
                    r = r - (e - es);
                } else {
                    /* step E to next utf8 chinese char */
                    i = i + 3;
                    r = r - 3;
                }
            } else {
                /*
                 * split non-utf8 chinese word also
                 */
                if ((*e & 0xE0) != 0xE0 ||
                    (*(e+1) & 0x80) != 0x80 ||
                    (*(e+2) & 0x80) != 0x80) {
                    i++;
                    r--;
                } else {
                    i += 3;
                    r -= 3;
                }
            }
        }

        /* step S to s's next char, or, next char after max word length founded */
        if (t > s) {
            if ((opt & EDIV_SOPT_ONLY_MAXMATCH) && cbk) cbk(s, t - s);
            s = t;
        } else {
            if ((*s & 0xE0) == 0xE0 &&
                (*(s+1) & 0x80) == 0x80 &&
                (*(s+2) & 0x80) == 0x80)
                r = 3;
            else r = 1;
            
            if (!(opt & EDIV_SOPT_THROW_UNKNOWN) && cbk) cbk(s, r);
            s = s + r;
        }
    }

    return STATUS_OK;
}
