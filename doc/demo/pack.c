#include "mheads.h"
#include "lheads.h"
#include "packet.h"
#include "timer.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv, char **envp)
{
	unsigned char buf[2048];
	int blen = 2048, len;
	char *s;
	HDF *hdf;
	unsigned long elapsed;
	
	mconfig_parse_file(PATH_SITE"/tpl/oms.hdf", &g_cfg);

	timer_start();
	for (int i = 0; i < 100000; i++) {
		memset(buf, 2048, 0x0);
		len = pack_hdf(g_cfg, buf, blen);
		unpack_hdf(buf, len, &hdf);
		s = hdf_get_value(hdf, "manual.Layout", NULL);
		hdf_destroy(&hdf);
	}
	elapsed = timer_stop();
	
 	printf("Time elapsed: %lu\n", elapsed);
	
	return 0;
}
