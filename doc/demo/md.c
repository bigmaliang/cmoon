#include "mheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv)
{
    char *buf = "1861017895712012/12/13 0:00:0011234n-2Sv$#xv%Mvz@";


    char out[LEN_MD5] = {0};

    mstr_md5_str(buf, out);

    printf("%s\n", out);
    
    return 0;
}
