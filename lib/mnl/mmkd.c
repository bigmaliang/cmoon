#include "mheads.h"

NEOERR* mkd_esc_str(const char *in, char **out)
{
    if (!in || !out) return STATUS_OK;

    int len = 0;
    *out = NULL;
    
    char *s = NULL;
    MMIOT *m = mkd_string((char*)in, strlen(in), 0);
    if (m) {
        if (mkd_compile(m, 0)) {
            len = mkd_document(m, &s);
            if (s && len > 0) {
                *(s+len) = '\0';
                *out = strdup(s);
            }
            mkd_cleanup(m);
        }
        return STATUS_OK;
    }

    return nerr_raise(NERR_SYSTEM, "mkd_string() error %s", in);
}
