/*
 * 即时数据备份工具
 * ==============
 *
 * 用途：
 * ----
 * 将社区qdbm中增量数据写入mysql数据库，并从nmdb中清理掉没有更新的、长时间没访问的数据
 *
 * 用法：
 * ----
 * 因为qdbm不能同时被多个进程打开，所以需要先拷贝一份最新的qdbm文件作为程序遍历的源到本机
 * 执行程序时需要指定qdbm数据库的文件名，和该文件对应的域(domain, 如n_blog)用来连接对应的nmdb
 * 具体方法请参考执行./storedb的输出帮助
 *
 * 原理：
 * ----
 * 遍历读取源qdbm数据库中的所有key, 从nmdb中取出key对应的值进行以下操作：
 * 1, 根据该key的辅助数据increment来判断该条记录是否有变化，如果需要的话，将值写入mysql数据库
 * 2, 根据该key的辅助数据timestamp来判断该记录最近被访问的时间是否在一个月之类，
 *    如果超过一个月，将该key, key_increment, key_timestamp从nmdb中删除，以保持nmdb的高效率。
 *    有increment且没有写入mysql的key不会删除。
 */
#define NFCGI
#include <depot.h>

#include "mheads.h"
#include "lheads.h"
#include "ocds.h"

void useage(char *prg)
{
	printf("usage: %s -f qdbm filename -d domain name\n"
		   "\t -f the qdbm database file name need to be stored.\n"
		   "\t -d domain the db stored for.\n"
		   "example:\n"
		   "\t %s -f /data/qdbm/backup/blog -d n_blog\n",
		   prg, prg);
	exit (1);
}

char g_dbfn[LEN_FN];
char g_domain[LEN_FN];

HDF *g_cfg = NULL;

int main(int argc, char *argv[])
{
	DEPOT *qdb = NULL;
	fdb_t *fdb = NULL;
	nmdb_t *nmdb = NULL;
	
	//char *key, *val, *v;
	char *key, *v;
	char val[LEN_NMDB_VAL];
	int increment;
	int lenkey;
	bool dboffered, dmoffered;
	size_t r;
	int ret;

	if (!mconfig_parse_file(CONFIG_FILE, &g_cfg)) {
		mtc_err("init config %s error", CONFIG_FILE);
		return 1;
	}
	mtc_init(HF_LOG_PATH"mtools.storedb");

	dboffered = dmoffered = false;
	if (argc > 1) {
		int c;
		while ( (c=getopt(argc, argv, "f:d:")) != -1 ) {
			switch (c) {
			case 'f':
				dboffered = true;
				strncpy(g_dbfn, optarg, sizeof(g_dbfn));
				break;
			case 'd':
				dmoffered = true;
				strncpy(g_domain, optarg, sizeof(g_domain));
				break;
			default:
				useage(argv[0]);
			}
		}
	} else {
		useage(argv[0]);
	}
	if (!dboffered || !dmoffered) {
		useage(argv[0]);
	}

	ret = fdb_init_long(&fdb, hdf_get_value(g_cfg, CFG_DB".ip", "127.0.0.1"),
						hdf_get_value(g_cfg, CFG_DB".user", "test"),
						hdf_get_value(g_cfg, CFG_DB".pass", "test"),
						hdf_get_value(g_cfg, CFG_DB".name", "test"),
						(unsigned int)hdf_get_int_value(g_cfg, CFG_DB".port", 0));
	if (ret != RET_DBOP_OK) {
		mtc_err("init db error");
		return 1;
	}
	
	nmdb = nmdb_init();
	if (nmdb == NULL) {
		mtc_err("init nmdb error");
		return 1;
	}
	ret = cds_add_udp_server(nmdb, g_domain);
	if (ret != RET_DBOP_OK) {
		mtc_err("add nmdb server for %s failure", g_domain);
		return 1;
	}

	bool repaired = false;
 reopen:
	if(!(qdb = dpopen(g_dbfn, DP_OREADER|DP_OWRITER, -1))) {
		if (!repaired) {
			mtc_warn("try to reapir %s", g_dbfn);
			dprepair(g_dbfn);
			repaired = true;
			goto reopen;
		}
		mtc_err("open %s failure %s", g_dbfn, dperrmsg(dpecode));
		return 1;
	}
	dpiterinit(qdb);
	
	char tkey[LEN_NMDB_KEY];
	time_t timenow = time(NULL);
	while ((key = dpiternext(qdb, &lenkey)) != NULL) {
		if (lutil_is_userdata_key(key)) {
			snprintf(tkey, sizeof(tkey), "%s_"POST_INCREMENT, key);
			r = nmdb_get(nmdb, (unsigned char*)tkey, strlen(tkey), (unsigned char*)val, LEN_NMDB_VAL);
			if ((int)r <= 0) {
				mtc_info("get %s failure", tkey);
				goto doclean;
			}
			*(val+r) = '\0';
			v = neos_strip(val);
			increment = atoi(v);
			if (increment > 0) {
				r = nmdb_get(nmdb, (unsigned char*)key, strlen(key), (unsigned char*)val, LEN_NMDB_VAL);
				if ((int)r <= 0) {
					mtc_err("%s increment found, but data not found, cleanup...", key);
					goto doclean;
				}
				*(val+r) = '\0';
				v = neos_strip(val);
				if (atoi(v) <= 0) {
					mtc_err("%s increment %d, but value %s", key, increment, v);
					goto doclean;
				}
				ret = cds_store_increment(fdb, key, v, increment);
				if (ret != RET_DBOP_OK) {
					mtc_err("store %s for %s to db failure %d", v, key, ret);
					if (ret == RET_DBOP_NEXIST) {
						nmdb_del(nmdb, (unsigned char*)key, lenkey);
						nmdb_del(nmdb, (unsigned char*)tkey, strlen(tkey));
						snprintf(tkey, sizeof(tkey), "%s_"POST_TIMESTAMP, key);
						nmdb_del(nmdb, (unsigned char*)tkey, strlen(tkey));
					}
					free(key);
					continue;
				}
				strcpy(val, "0");
				r =  nmdb_set(nmdb, (unsigned char*)tkey, strlen(tkey), (unsigned char*)val, strlen(val)+1);
			}

		doclean:
			/* 删掉最近一月前才有读取的key */
			snprintf(tkey, sizeof(tkey), "%s_"POST_TIMESTAMP, key);
			r = nmdb_get(nmdb, (unsigned char*)tkey, strlen(tkey), (unsigned char*)val, LEN_NMDB_VAL);
			if ((int)r <= 0) {
				mtc_info("get %s failure", tkey);
				free(key);
				continue;
			}
			*(val+r) = '\0';
			/* time_t defined in bits/types.h #define __SLONGWORD_TYPE        long int */
			if (atol(val) <= timenow-(60*60*24*30)) {
				nmdb_del(nmdb, (unsigned char*)key, lenkey);
				nmdb_del(nmdb, (unsigned char*)tkey, strlen(tkey));
				snprintf(tkey, sizeof(tkey), "%s_"POST_INCREMENT, key);
				nmdb_del(nmdb, (unsigned char*)tkey, strlen(tkey));
			}
		}
		free(key);
	}
	
	dpclose(qdb);
	fdb_free(&fdb);
	if (nmdb != NULL)
		nmdb_free(nmdb);
	
	return 0;
}
