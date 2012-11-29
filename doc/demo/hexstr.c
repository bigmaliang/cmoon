#include "mheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv)
{
    char *s = "764839acf2d1";
    uint8_t u[6];
    char outs[20];

    mstr_str2hex(s, strlen(s), u);

    mstr_hex2str(u, 6, outs);

    printf("ori: %s, convert: %s\n", s, outs);
    
    return 0;
}
