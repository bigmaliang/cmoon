#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ClearSilver.h>

int main()
{
    HDF *hdf;

    hdf_init(&hdf);

    hdf_set_valuef(hdf, "%s.ddd=yyyyy", "xxx");

    hdf_dump(hdf, NULL);

    return 0;
}
