/*
 * 即时数据存取接口
 * ==============
 *
 * 参数:
 * ----
 * op
 * 指定数据的域，当前可以为
 * 	"n_user_space",
 * 	"n_photo", "n_photo_album",
 * 	"n_video", "n_video_d", "n_video_album",
 * 	"n_blog"
 * 多个值时以','隔开。 (当前只支持同时取多个，设置和自增不支持多key)
 *
 * key
 * 指定key, key值必须以','隔开的整数，个数必须与op的个数一致。(当前只支持同时取多个，设置和自增不支持多key)
 *
 * query
 * 为便于调试，可以在url中指定获取方式, post为设置, put为自增, 默认为获取
 * (使用post 或者 put请求能达到相同的效果)
 *
 * val
 * query=post时必选参数，将key设置成val
 *
 * inc
 * query=put时可选参数，指定自增值，默认为加1
 *
 * jsoncallback
 * 为解决跨域访问问题加入的参数，当这个参数设置时，接口的输出不是json数据，而是执行回调函数，以json数据为参数
 * 如，jsoncallback=xxx, 接口最后会调用 xxx({"success": "1", key: XXX, value: XXX});
 *
 *
 * 实例：
 * ----
 * 	http://rtds.dsc.hunantv.com/cgi-bin/cds.cgi?op=n_blog,n_blog,n_video&key=1,3,5
 * 	获取博文id为1,3的文章的浏览次数，视频id为5的视频的播放次数
 * 		
 * 	http://rtds.dsc.hunantv.com/cgi-bin/cds.cgi?op=n_blog&key=1&query=post&val=100
 * 	将博文id为1的文章的浏览次数设置为100
 *
 * 	http://rtds.dsc.hunantv.com/cgi-bin/cds.cgi?op=n_blog&key=1&query=put&inc=10
 * 	将博文id为1的文章的浏览次数加10
 *
 *
 * 返回:
 * ----
 * 	以json格式返回操作结果。
 * 	操作成功时会设置{"success": "1", key: XXX, value: XXX}， (其中XXX为整形数值)
 *  获取多个key时返回值中value是个对象，例如：
 *  "value": [ { "op": "n_blog", "key": 18, "value": 5809 }, { "op": "n_video", "key": 4999, "errmsg": "查询资源不存在" } ]
 *  key为传入的key参数。value为key当前最新的值。
 * 	失败时会设置{"errmsg": "XXX", key: XXX}，并指定失败原因。key为传入参数。
 */
#include "mheads.h"
#include "lheads.h"
#include "ocds.h"

HDF *g_cfg = NULL;

int main(int argc, char **argv, char **envp)
{
	char key[LEN_NMDB_KEY], val[LEN_NMDB_VAL], tkey[LEN_NMDB_KEY], hdfkey[LEN_ST], thdfkey[LEN_ST];
	ULIST *keylist = NULL, *domainlist = NULL;
	char *k, *v, *p, *user, *pass;
	char timenow[LEN_LONG];
	int64_t inc, incr;
	
	CGI *cgi = NULL;
	NEOERR *err;
	nmdb_t *db = NULL;
	fdb_t *fdb = NULL;
	size_t r;
	int i, op = CGI_REQ_GET;
	int ret = 0;
	time_t tm;

	//sleep(20);
	mutil_wrap_fcgi(argc, argv, envp);

	if (!mconfig_parse_file(CONFIG_FILE, &g_cfg)) {
		mtc_err("init config %s error", CONFIG_FILE);
		printf("Content-Type: text/html; charset=UTF-8\r\n\r\n");
		printf("{errmsg: \"初始化配置失败\"}");
		return 1;
	}
	mtc_init(HF_LOG_PATH"cds");
	
	ret = fdb_init_long(&fdb, hdf_get_value(g_cfg, CFG_DB".ip", "127.0.0.1"),
						hdf_get_value(g_cfg, CFG_DB".user", "test"),
						hdf_get_value(g_cfg, CFG_DB".pass", "test"),
						hdf_get_value(g_cfg, CFG_DB".name", "test"),
						(unsigned int)hdf_get_int_value(g_cfg, CFG_DB".port", 0));
	if (ret != RET_DBOP_OK) {
		mtc_err("init db error %s", fdb_error(fdb));
		printf("Content-Type: text/html; charset=UTF-8\r\n\r\n");
		printf("{errmsg: \"初始化数据库失败\"}");
		return 1;
	}
	
	while (FCGI_Accept() >= 0) {
		/*
		 * 获取参数
		 */
		db = NULL;			/* nmdb_free 该db会造成fastcgi core 掉。。。 */
		keylist = domainlist = NULL;
		err = cgi_init(&cgi, NULL);
		if (err != STATUS_OK) {
			mtc_err("init cgi error");
			printf("Content-Type: text/html; charset=UTF-8\r\n\r\n");
			printf("{errmsg: \"初始化失败\"}");
			goto opfinish;
		}
		err = cgi_parse(cgi);
		if (err != STATUS_OK) {
			mtc_err("parse cgi error");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "初始化错误");
			goto opfinish;
		}
		if (mutil_client_attack_cookie(cgi->hdf, "cds", 30, 60)) {
			mtc_err("client attack");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "需要休息");
			goto opfinish;
		}
		
		char *domain = hdf_get_value(cgi->hdf, PRE_QUERY".op", NULL);
		k = hdf_get_value(cgi->hdf, PRE_QUERY".key", NULL);
		if (k == NULL) {
			mtc_warn("no parameter: key");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "缺少 key 参数");
			goto opfinish;
		}
		hdf_set_copy(cgi->hdf, PRE_OUTPUT".key", PRE_QUERY".key");
		tm = time(NULL);


		/* 解析 key */
		ret = cds_parse_key(k, &keylist);
		if (ret != RET_DBOP_OK || uListLength(keylist) < 1) {
			mtc_err("key %s illegal", k);
			hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "key 格式不正确");
			goto opfinish;
		}
		uListGet(keylist, 0, (void**)&p);
		snprintf(key, sizeof(key), "%s_%s", domain, p);
		/* 解析 domain */
		ret = cds_parse_domain(domain, &domainlist);
		if (ret != RET_DBOP_OK || uListLength(domainlist) != uListLength(keylist)) {
			mtc_err("domain %s and key %s illegal", domain, k);
			hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "op 格式不正确");
			goto opfinish;
		}
		db = nmdb_init();
		if (db == NULL) {
			mtc_err("init nmdb error");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "初始化失败");
			goto opfinish;
		}
		uListGet(domainlist, 0, (void**)&p);
		ret = cds_add_udp_server(db, p);
		if (ret != RET_DBOP_OK) {
			mtc_err("add nmdb server for %s failure", p);
			hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "op 参数无效");
			goto opfinish;
		}

		/*
		 * 开始干活
		 */
		op = CGI_REQ_METHOD(cgi);
		if (!strcmp(hdf_get_value(cgi->hdf, PRE_QUERY".query", "unknown"), "post")) op = CGI_REQ_POST;
		else if (!strcmp(hdf_get_value(cgi->hdf, PRE_QUERY".query", "unknown"), "put")) op = CGI_REQ_PUT;
		switch (op) {
		case CGI_REQ_GET:
			for (i = 0; i < uListLength(keylist); i++) {
				uListGet(domainlist, i, (void**)&domain);
				uListGet(keylist, i, (void**)&p);
				snprintf(key, sizeof(key), "%s_%s", domain, p);
				//snprintf(hdfkey, sizeof(hdfkey), "%s.value.%s", PRE_OUTPUT, p);
				snprintf(hdfkey, sizeof(hdfkey), "%s.value.%d", PRE_OUTPUT, i);
				hdf_set_valuef(cgi->hdf, "%s.op=%s", hdfkey, domain);
				hdf_set_valuef(cgi->hdf, "%s.key=%s", hdfkey, p);
				snprintf(thdfkey, sizeof(thdfkey), "%s.key", hdfkey);
				hdf_set_attr(cgi->hdf, thdfkey, "type", "int");
				if (db != NULL) nmdb_free(db);
				db = nmdb_init();
				ret = cds_add_udp_server(db, domain);
				if (ret != RET_DBOP_OK) {
					mtc_warn("add domain %s failure", domain);
					hdf_set_valuef(cgi->hdf, "%s.errmsg=不支持op", hdfkey);
					db = NULL;			/* nmdb_free 该db会造成fastcgi core 掉。。。 */
					continue;
				}
			
				memset(val, 0x0, sizeof(val));
				snprintf(tkey, sizeof(tkey), "%s_"POST_TIMESTAMP, key);
				snprintf(timenow, sizeof(timenow), "%lu", tm);
				r = nmdb_get(db, (unsigned char*)key, strlen(key), (unsigned char*)val, LEN_NMDB_VAL);
				if ((int)r <= 0) {
					mtc_dbg("%s not in nmdb, get from db...", key);
					ret = cds_get_data(cgi->hdf, p, domain, hdfkey, fdb);
					if (ret == RET_DBOP_OK) {
						v = hdf_get_valuef(cgi->hdf, "%s.value", hdfkey);
						if (v != NULL) {
							nmdb_set(db, (unsigned char*)key, strlen(key), (unsigned char*)v, strlen(v)+1);
							/* 更新该key的timestamp记录，为减少网络压力，10次读取1次更新 */
							if (tm % 10 == 0)
								nmdb_set(db, (unsigned char*)tkey, strlen(tkey), (unsigned char*)timenow, strlen(timenow)+1);
							break;
						}
					}
					mtc_warn("%s get from db failure!", key);
				}
				if ((int)r == -2) {
					hdf_set_valuef(cgi->hdf, "%s.errmsg=查询失败", hdfkey);
					continue;
				} else if ((int)r == -1) {
					hdf_set_valuef(cgi->hdf, "%s.errmsg=查询资源不存在", hdfkey);
					continue;
				} else {
					*(val+r) = '\0';
					/* nmdb_incr() 会自作聪明的把val按23个字符右对齐 */
					v = neos_strip(val);
					mtc_dbg("%s get from nmdb ok %s", key, v);
					hdf_set_valuef(cgi->hdf, "%s.value=%s", hdfkey, v);
					snprintf(thdfkey, sizeof(thdfkey), "%s.value", hdfkey);
					hdf_set_attr(cgi->hdf, thdfkey, "type", "int");
					if (tm % 10 == 0)
						nmdb_set(db, (unsigned char*)tkey, strlen(tkey), (unsigned char*)timenow, strlen(timenow)+1);
				}
			}
			break;
		case CGI_REQ_POST:
			user = hdf_get_value(cgi->hdf, PRE_QUERY".user", NULL);
			pass = hdf_get_value(cgi->hdf, PRE_QUERY".pass", NULL);
			if (!lutil_user_has_power(user, pass)) {
				mtc_err("%s %s not authered", user, pass);
				hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "设置操作错误");
				ret = -1; goto opfinish;
			}
			v = hdf_get_value(cgi->hdf, PRE_QUERY".val", NULL);
			if (v == NULL) {
				mtc_warn("no parameter: val");
				hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "缺少 val 参数");
				goto opfinish;
			}
			mtc_foo("welcome %s for set %s to %s", user, key, v);
			hdf_set_copy(cgi->hdf, PRE_OUTPUT".value", PRE_QUERY".val");
			hdf_set_attr(cgi->hdf, PRE_OUTPUT".key", "type", "int");
			hdf_set_attr(cgi->hdf, PRE_OUTPUT".value", "type", "int");
			r =  nmdb_set(db, (unsigned char*)key, strlen(key), (unsigned char*)v, strlen(v)+1);
			if (r < 0) {
				mtc_err("set into nmdb failure!");
				hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "设置操作失败");
				ret = -1; goto opfinish;
			}
			/* 更新该key的increment记录 */
			snprintf(tkey, sizeof(tkey), "%s_"POST_INCREMENT, key);
			nmdb_set(db, (unsigned char*)tkey, strlen(tkey), (unsigned char*)v, strlen(v)+1);
			mtc_dbg("set into nmdb ok");
			break;
		case CGI_REQ_PUT:
			/*
			 * 增加操作，同时设置key_POST_INCREMENT
			 * 当nmdb中没有相关key时，设值为inc，后续同步时能写入mysql
			 */
			v = hdf_get_value(cgi->hdf, PRE_QUERY".incNEXIST", "1");
			//inc = (int64_t)hdf_get_int_value(cgi->hdf, PRE_QUERY".inc", 1);
			inc = 1;
			
			r = nmdb_incr(db, (unsigned char*)key, strlen(key), inc, &incr);
			if (r == 0) {
				mtc_warn("%s not match", key);
				//hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "无匹配资源");
				//goto opfinish;
				r = nmdb_set(db, (unsigned char*)key, strlen(key), (unsigned char*)v, strlen(v)+1);
				if (r < 0) {
					mtc_err("set into nmdb failure!");
					hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "设置失败");
					ret = -1; goto opfinish;
				}
				incr = 1;
			} else if (r == 1) {
				mtc_warn("%s unincrementable", key);
				hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "值不可加");
				goto opfinish;
			} else if (r != 2) {
				mtc_warn("%s inc %d failure", key, inc);
				hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "增加操作失败");
				ret = -1; goto opfinish;
			}
			hdf_set_int_value(cgi->hdf, PRE_OUTPUT".value", (int)incr);
			hdf_set_attr(cgi->hdf, PRE_OUTPUT".key", "type", "int");
			hdf_set_attr(cgi->hdf, PRE_OUTPUT".value", "type", "int");

			/* 更新该key的increment记录 */
			snprintf(tkey, sizeof(tkey), "%s_"POST_INCREMENT, key);
			r = nmdb_incr(db, (unsigned char*)tkey, strlen(tkey), inc, &incr);
			if (r == 0) {
				mtc_warn("%s not match", tkey);
				nmdb_set(db, (unsigned char*)tkey, strlen(tkey), (unsigned char*)v, strlen(v)+1);
			}
			break;
		default:
			mtc_warn("unsupport operation");
			hdf_set_value(cgi->hdf, PRE_OUTPUT".errmsg", "操作类型不支持");
			goto opfinish;
		}

		hdf_set_value(cgi->hdf, PRE_OUTPUT".success", "1");

	opfinish:
		if (db != NULL)	nmdb_free(db);
		if (keylist != NULL) uListDestroy(&keylist, ULIST_FREE);
		if (domainlist != NULL) uListDestroy(&domainlist, ULIST_FREE);
		if (cgi != NULL) {
			char *cb = hdf_get_value(cgi->hdf, PRE_QUERY".jsoncallback", NULL);
			if (cb != NULL) {
				if (op == CGI_REQ_GET)
					mjson_execute_hdf(cgi->hdf, cb, CDS_CACHE_SECOND);
				else
					mjson_execute_hdf(cgi->hdf, cb, 0);
			} else {
				if (op == CGI_REQ_GET)
					mjson_output_hdf(cgi->hdf, CDS_CACHE_SECOND);
				else
					mjson_output_hdf(cgi->hdf, 0);
			}
#ifdef DEBUG_HDF
			hdf_write_file(cgi->hdf, HF_LOG_PATH"hdf.cds");
#endif
			cgi_destroy(&cgi);
		}
 	}
 
	fdb_free(&fdb);
	mconfig_cleanup(&g_cfg);
	return 0;
}
