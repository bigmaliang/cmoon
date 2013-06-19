#include "mheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv)
{
    char *buf = "1861017895712012/12/13 0:00:0011234n-2Sv$#xv%Mvz@";
    int iarr[100];

    for (int x = 0; x < 100; x++) {
        printf("%d\t", iarr[x]);
    }

    mutil_rand_numbers(100, iarr, 100);

    printf("\n\n\n");

    for (int x = 0; x < 100; x++) {
        printf("%d\t", iarr[x]);
    }


    char out[LEN_MD5] = {0};

    mstr_md5_str(buf, out);

    //printf("%s %d\n", out, 22 % atoi(argv[1]));
    
    return 0;
}
