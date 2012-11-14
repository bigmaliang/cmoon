#include "mheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv, char **envp)
{
    char s[99] = "和你ME";

    char *a, *b;

    printf("%s\n", s);

    neos_url_escape(s, &a, NULL);
    printf("%s\n", a);
    
    neos_unescape(a, strlen(a), '%');
    printf("%s\n", a);

    
    return 0;
}
