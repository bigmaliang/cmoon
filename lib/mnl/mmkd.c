#include "mheads.h"

NEOERR* mkd_esc_str(const char *in, char **out)
{
    if (!in || !out) return STATUS_OK;

    *out = NULL;
    
    char *s;
    MMIOT *m = mkd_string((char*)in, strlen(in), 0);
    if (m) {
        if (mkd_compile(m, 0)) {
            mkd_document(m, &s);
            if (s) *out = strdup(s);
            mkd_cleanup(m);
        }
        return STATUS_OK;
    }

    return INTERNAL_ERR;
}
