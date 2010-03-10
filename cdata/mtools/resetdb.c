/*
 * 即时数据清0工具
 * =============
 *
 * 用途：
 * ----
 * 将社区 mysql 数据库中 view_num_week 数据清 0
 */
#define NFCGI
#include "mheads.h"
#include "lheads.h"
#include "ocds.h"

void useage(char *prg)
{
	printf("usage: %s -f qdbm filename -d domain name\n"
		   "\t -f force counter reset.\n"
		   "\t -w reset the view_num_week counter.\n"
		   "\t -m reset the view_num_month counter.\n"
		   "example:\n"
		   "\t %s -f -w\n",
		   prg, prg);
	exit (1);
}

HDF *g_cfg = NULL;

int main(int argc, char *argv[])
{
	fdb_t *fdb = NULL;
	int ret;

	bool force, week, month;
	force = week = month = false;

	if (argc > 1) {
		int c;
		while ((c=getopt(argc, argv, "fwm")) != -1) {
			switch (c) {
			case 'f':
				force = true;
				break;
			case 'w':
				week = true;
				break;
			case 'm':
				month = true;
				break;
			default:
				useage(argv[0]);
			}
		}
	} else {
		useage(argv[0]);
	}

	time_t tm = time(NULL);
	struct tm stm;
	localtime_r(&tm, &stm);

	if (week && stm.tm_wday != 0 && !force) {
		printf("today is %d, not sunday, use -f to force reset.\n", stm.tm_wday);
		exit(2);
	}

	if (month && stm.tm_mday != 1 && !force) {
		printf("today is %d, not 1st day, use -f to force reset.\n", stm.tm_mday);
		exit(2);
	}

	if (!mconfig_parse_file(CONFIG_FILE, &g_cfg)) {
		mtc_err("init config %s error", CONFIG_FILE);
		return 1;
	}
	mtc_init(HF_LOG_PATH"mtools.resetdb");
	
	ret = fdb_init_long(&fdb, hdf_get_value(g_cfg, CFG_DB".ip", "127.0.0.1"),
						hdf_get_value(g_cfg, CFG_DB".user", "test"),
						hdf_get_value(g_cfg, CFG_DB".pass", "test"),
						hdf_get_value(g_cfg, CFG_DB".name", "test"),
						(unsigned int)hdf_get_int_value(g_cfg, CFG_DB".port", 0));
	if (ret != RET_DBOP_OK) {
		mtc_err("init db error");
		return 1;
	}

	if (week) {
		cds_reset_increment(fdb, "view_num_week");
	}
	if (month) {
		cds_reset_increment(fdb, "view_num_month");
	}

	fdb_free(&fdb);
	return 0;
}
