#include "mheads.h"
#include "lheads.h"

int g_get = 0;
int g_store = -1;
int g_count = -1;
time_t g_exp = 10;
char g_key[LEN_SM];
char g_val[LEN_SM];

HDF *g_cfg;

void useage(char *prg)
{
        printf("usage: %s -g[s|a|r][i|d] key[=value][:flags] -e ExpireTime\n"
               "\t -g get value for key\n"
               "\t -s set value for key. default expire time is %lus\n"
               "\t -a add value for key.\n"
               "\t -r replace value for key.\n"
               "\t -i increment value for key.\n"
               "\t -d decrement value for key.\n"
               "example:\n"
               "\t %s -g PaGPQBkJ\n",
               prg, g_exp, prg);
        exit(1);
}

int main(int argc, char *argv[])
{
	mtc_init(TC_ROOT"memop_mtl");

        if (argc > 1) {
                int c;
                while ( (c=getopt(argc, argv, "g:s:a:r:i:d:e:")) != -1 ) {
                        switch(c) {
                        case 'g':
				g_get = 1;
                                strncpy(g_key, optarg, sizeof(g_key)-1);
                                break;
                        case 's':
				g_store = MMC_OP_SET;
                                strncpy(g_key, optarg, sizeof(g_key)-1);
                                break;
                        case 'a':
				g_store = MMC_OP_ADD;
                                strncpy(g_key, optarg, sizeof(g_key)-1);
                                break;
                        case 'r':
				g_store = MMC_OP_REP;
                                strncpy(g_key, optarg, sizeof(g_key)-1);
                                break;
                        case 'i':
				g_store = MMC_OP_INC;
                                strncpy(g_key, optarg, sizeof(g_key)-1);
                                break;
                        case 'd':
				g_store = MMC_OP_DEC;
                                strncpy(g_key, optarg, sizeof(g_key)-1);
                                break;
			case 'e':
				g_exp = atoi(optarg);
				break;
                        default:
                                useage(argv[0]);
                        }
                }
        } else
                useage(argv[0]);
	
	memcached_return rc;
	if (g_store >= MMC_OP_SET) {
		uint32_t flg;
		char *p;
		p = strchr(g_key, '=');
		if (p != NULL) {
			strncpy(g_val, p+1, sizeof(g_val)-1);
			*p = '\0';
			p = strchr(g_val, ':');
			if (p != NULL) {
				*p = '\0';
				flg = atoi(p+1);
			} else {
				flg = 0;
			}
		} else {
			printf("set key need value\n");
			exit(1);
		}
		rc = mmc_store(g_store, g_key, g_val, 0, g_exp, flg);
		if (rc == MEMCACHED_SUCCESS) {
			printf("set %s to %s:%d exp=%lu ok\n",
			       g_key, g_val, flg, g_exp);
		} else {
			printf("set %s to %s:%d exp=%lu failure. pls check log\n",
			       g_key, g_val, flg, g_exp);
		}
	}
	if (g_count >= MMC_OP_SET) {
		uint32_t flg;
		char *p;
		p = strchr(g_key, '=');
		if (p != NULL) {
			strncpy(g_val, p+1, sizeof(g_val)-1);
			*p = '\0';
			p = strchr(g_val, ':');
			if (p != NULL) {
				*p = '\0';
				flg = atoi(p+1);
			} else {
				flg = 0;
			}
		} else {
			printf("count key need offset\n");
			exit(1);
		}
		uint64_t count;
		rc = mmc_count(g_count, g_key, atoi(g_val), &count, g_exp, flg);
		if (rc == MEMCACHED_SUCCESS) {
			printf("count %s to %s:%d exp=%lu ok. cur count %u\n",
			       g_key, g_val, flg, g_exp, count);
		} else {
			printf("set %s to %s:%d exp=%lu failure. pls check log\n",
			       g_key, g_val, flg, g_exp);
		}
	}
	if (g_get) {
		uint32_t flag;
		size_t len;
		char *val = mmc_get(g_key, &len, &flag);
		if (val != NULL) {
			printf("%s=%s:len %d:flag %d\n", g_key, val, len, flag);
		} else {
			printf("%s not in %s:%s\n", g_key, MEMC_IP, MEMC_PORT);
		}
	}

	return 0;
}
