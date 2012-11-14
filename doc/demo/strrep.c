#include "mheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv, char **envp)
{
#if 0
    char *s = argv[1], *f = argv[2], *t = argv[3];

    printf("strstr %s from %s to %s return: %s\n",
           s, f, t, mmisc_str_repstr(s, f, t));


    char *s = "hei,1122abcde22";
    printf("strstr %s return: %s\n",
           s, mmisc_str_repstr(s, "11", "xx", "22", "wori"));
#endif

    return 0;
}
