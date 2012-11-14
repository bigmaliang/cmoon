#include "mheads.h"
#include "eheads.h"

HDF *g_cfg = NULL;

static void div_cbk(char *s, size_t len)
{
    char tok[1024];

    snprintf(tok, len+1, s);
    mtc_foo("%s", tok);
}


int main(int argc, char **argv, char **envp)
{
    char *bm = NULL;
	NEOERR *err;
    int cnt = 0;

    char *s = argv[1];
    
    mtc_init("div");

    err = ediv_init_from_file(&bm, "/home/bigml/web/moon/eii/data/dict.txt", &cnt);
    DIE_NOK_MTL(err);

    mtc_foo("%u words seted\n", cnt);

    //mtc_foo("%u distinct word\n", ediv_word_distinct(bm));

    ediv_word_split(bm, s, strlen(s), NULL, div_cbk, EDIV_SOPT_ONLY_MAXMATCH |
        EDIV_SOPT_SKIP_NUTF);

    free(bm);
    
    return 0;
}
