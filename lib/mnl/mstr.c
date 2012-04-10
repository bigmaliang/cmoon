#include "mheads.h"

void mstr_rand_string(char *s, int max)
{
    int size;
    int x = 0;

    size = neo_rand(max-1);
    for (x = 0; x < size; x++)
    {
        s[x] = (char)(65 + neo_rand(90-65));
    }
    s[x] = '\0';
}

void mstr_html_escape(HDF *node, char *name)
{
    if (!node || !name) return;
    char *s, *os;
    
    while (node) {
        s = hdf_get_value(node, name, NULL);
        if (s) {
            neos_html_escape(s, strlen(s), &os);
            hdf_set_value(node, name, os);
            free(os);
        }
        
        node = hdf_obj_next(node);
    }
}

void mstr_html_unescape(HDF *node, char *name)
{
    if (!node || !name) return;
    char *s, *os;
    
    while (node) {
        s = hdf_get_value(node, name, NULL);
        if (s && *s) {
            os = mstr_repstr(3, s,
                             "&amp;", "&",
                             "&gt;", ">",
                             "&lt;", "<");
            if (os) {
                hdf_set_value(node, name, os);
                free(os);
            }
        }
        
        node = hdf_obj_next(node);
    }
}

void mstr_script_escape(HDF *node, char *name)
{
    if (!node || !name) return;
    char *s, *os;
    
    while (node) {
        s = hdf_get_value(node, name, NULL);
        if (s && *s) {
            os = mstr_repstr(2, s,
                             "<script", "&lt;script",
                             "</script>", "&lt;/script&gt;");
            if (os) {
                hdf_set_value(node, name, os);
                free(os);
            }
        }
        
        node = hdf_obj_next(node);
    }
}

void mstr_md5_buf(unsigned char *in, size_t len, char out[LEN_MD5])
{
    if (!in) return;
    
    md5_ctx my_md5;
    unsigned char hexres[16];

    MD5Init(&my_md5);
    MD5Update(&my_md5, in, (unsigned int)len);
    memset(hexres, 0x0, 16);
    MD5Final(hexres, &my_md5);
    
    mstr_hex2str(hexres, 16, (unsigned char*)out);
}

void mstr_md5_str(char *in, char out[LEN_MD5])
{
    return mstr_md5_buf((unsigned char*)in, strlen(in), out);
}

bool mstr_isdigit(char *s)
{
    if (s == NULL) return false;
    
    char *p = s;
    while (*p != '\0') {
        if (!isdigit((int)*p))
            return false;
        p++;
    }
    return true;
}

bool mstr_isdigitn(char *buf, size_t len)
{
    if (buf == NULL || len <= 0) return false;

    size_t cnt = 0;
    while (cnt < len) {
        if (!isdigit((int)*(buf+cnt)))
            return false;
        cnt++;
    }
    return true;
}

/* 2 -28 */
bool mstr_israngen(char *buf, size_t len, int *left, int *right)
{
    if (buf == NULL || len <= 0) return false;

    size_t cnt = 0;
    int min = 0, max = 0;
    
    while (cnt < len) {
        if (*(buf+cnt) == '-') {
            if (cnt == 0 || cnt == len-1) return false;

            min = atoi(buf);
            max = atoi(buf+cnt+1);
            if (min >= max) return false;
            
            if (left) *left = min;
            if (right) *right = max;
            return true;
        }

        cnt++;
    }
    return false;
}

void mstr_real_escape_string(char *to, char *from, size_t len)
{
    char escape = 0;
    
    for (size_t i = 0; i < len; i++) {
        escape = 0;
        switch (*(from+i)) {
        case 0:                             /* Must be escaped for 'mysql' */
            escape = '0';
            break;
        case '\n':                          /* Must be escaped for logs */
            escape = 'n';
            break;
        case '\r':
            escape = 'r';
            break;
        case '\\':
            escape = '\\';
            break;
        case '\'':
            escape = '\'';
            break;
        case '"':                           /* Better safe than sorry */
            escape = '"';
            break;
        case '\032':                        /* This gives problems on Win32 */
            escape = 'Z';
            break;
        case ';':
            escape = ';';
            break;
        }
        if (escape) {
            *to++ = '\\';
            *to++= escape;
        } else {
            *to++= *(from+i);
        }
    }
}

char* mstr_real_escape_string_nalloc(char **to, char *from, size_t len)
{
    if (!to || !from) return NULL;

    char *s = calloc(1, len*2+4);
    if (!s) return NULL;

    mstr_real_escape_string(s, from, len);
    *to = s;

    return s;
}

void mstr_repchr(char *s, char from, char to)
{
    if (!s) return;

    while (*s) {
        if (*s == from) *s = to;
        s++;
    }
}

char* mstr_repstr(int rep_count, char *s, ...)
{
    if (!s || rep_count <= 0) return NULL;

    STRING str, tstr;
    char *from, *to, *p, *q;
    size_t len;

    string_init(&str);
    string_init(&tstr);

    string_set(&tstr, s);
    
    va_list ap;
    va_start(ap, s);
    for (int i = 0; i < rep_count; i++) {
        from = va_arg(ap, char*);
        to = va_arg(ap, char*);

        string_clear(&str);
        len = strlen(from);
        q = tstr.buf;
        p = strstr(q, from);
        while (p) {
            string_appendn(&str, q, p-q);
            string_append(&str, to);
            
            q = p + len;
            p = strstr(q, from);
        }
        if (*q) string_append(&str, q);

        string_clear(&tstr);
        string_set(&tstr, str.buf);
    }
    va_end(ap);

    string_clear(&tstr);

    return str.buf;
}

char* mstr_strip (char *s, char n)
{
  int x;

  x = strlen(s) - 1;
  while (x>=0 && s[x]==n) s[x--] = '\0';

  while (*s && *s==n) s++;
  
  return s;
}

unsigned int hash_string(const char *str)
{
        int hash = 5381; // DJB Hash
        const char *s;
    
        for (s = str; *s != '\0'; s++) {
                hash = ((hash << 5) + hash) + tolower(*s);
        }
    
        return (hash & 0x7FFFFFFF);
}

unsigned int hash_string_rev(const char *str)
{
    int hash = 5381;
    int x = strlen(str) - 1;
    
    while (x >= 0 && str[x] != '\0')
        hash = ((hash << 5) + hash) + tolower(str[x--]);
    
    return (hash & 0x7FFFFFFF);
}

/*
 * use < 10 judgement, or, you can use array ['0', '1', ..., 'e', 'f']
 */
void mstr_hex2str(unsigned char *hexin, unsigned int inlen, unsigned char *charout)
{
    /* 48 '0' */
    /* 97 'a'  122 'z'  65 'A' */
#define HEX2STR(in, out)                        \
    do {                                        \
        if (((in) & 0xf) < 10) {                \
            (out) = ((in)&0xf) + 48;            \
        } else {                                \
            (out) = ((in)&0xf) - 10 + 97;        \
        }                                        \
    } while (0)

    if (hexin == NULL || charout == NULL)
        return;

    unsigned int i, j;
    memset(charout, 0x0, inlen*2+1);

    for (i = 0, j = 0; i < inlen; i++, j += 2) {
        HEX2STR(hexin[i]>>4, charout[j]);
        HEX2STR(hexin[i], charout[j+1]);
    }

    charout[j+1] = '\0';
}

void mstr_bin2char(unsigned char *in, unsigned int inlen, unsigned char *out)
{
    /* 48 '0' */
    /* 97 'a'  122 'z'  65 'A' */
#define HEX2STR(in, out)                        \
    do {                                        \
        if (((in) & 0xf) < 10) {                \
            (out) = ((in)&0xf) + 48;            \
        } else {                                \
            (out) = ((in)&0xf) - 10 + 97;        \
        }                                        \
    } while (0)

    if (in == NULL || out == NULL)
        return;

    unsigned int i, j;
    memset(out, 0x0, inlen*2+1);

    for (i = 0, j = 0; i < inlen; i++) {
        if (in[i] == 9 || in[i] == 10 ||
            (in[i] > 31 && in[i] < 127)) {
            /*
             * resolve printable charactors
             * see man ascii
             */
            out[j] = in[i];
            j++;
        } else {
            HEX2STR(in[i]>>4, out[j]);
            HEX2STR(in[i], out[j+1]);
            j += 2;
        }
    }

    out[j+1] = '\0';
}
