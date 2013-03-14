#include "mheads.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv, char **envp)
{
    int total = 27;

    int body[5] = {4, 14, 3, 5, 1};

    for (int i = 0; i < 5; i++) {
        int x = (int) (((float)body[i] /total + 0.005) * 100);
        float y = (float)body[i] / total * 100;
        printf("%d / 27  is %f %d\n", body[i], y, x);
    }
    
    return 0;
}
