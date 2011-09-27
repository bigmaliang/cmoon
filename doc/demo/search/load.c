#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "index.h"

int main(int argc, char *argv[])
{
    struct index *idx;

    printf("load\n");
    idx = index_load("index", (10 * 1024 * 1024), INDEX_LOAD_NOOPT, NULL);
    if (!idx) {
        printf("failure\n");
        return 1;
    }

    printf("search\n");
    struct index_result *result = malloc(sizeof(*result) * 10);
    unsigned int rnum;
    double tnum;
    int est;
    struct index_search_opt sopt = {
        .u.okapi_k3.k1 = 1.2F,
        .u.okapi_k3.k3 = 1e10,
        .u.okapi_k3.b = 0.75,
        .word_limit = 24817184,
        .accumulator_limit = 32767,
        .summary_type = INDEX_SUMMARISE_PLAIN
    };
    if (index_search(idx, "bar", 0, 10, result, &rnum, &tnum, &est,
                     INDEX_SEARCH_SUMMARY_TYPE, &sopt)) {
        for (int i = 0; i < rnum; i++) {
            printf("%d docno=%lu\n", i, result[i].docno);
            printf("%d id=%s\n", i, result[i].auxilliary);
            printf("%d score=%f\n", i, result[i].score);
            printf("%d summary=%s\n", i, result[i].summary);
        }
    } else {
        printf("failure\n");
        return 1;
    }

    return 0;
}
