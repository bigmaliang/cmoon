#include "mheads.h"

typedef struct _s_config s_config;
struct _s_config{
	char key[CFG_ITEM_NUM][CFG_ITEM_LEN];
	char value[CFG_ITEM_NUM][CFG_ITEM_LEN];
	int num;
};

static s_config *g_cfgs = NULL;
static char g_fn[LEN_FN] = "";

static bool cfg_init()
{
	int shmid;
	bool create = false;

	shmid = shmget(CFG_SHMEM_KEY, 0, SHM_MODE);
	if (shmid == -1) {
		/*
		 * don't trace anything here(before create share mem)
		 * or will be loooop forever
		 */
		shmid = shmget(CFG_SHMEM_KEY, sizeof(s_config), SHM_MODE|IPC_CREAT);
		if (shmid == -1) {
			mtc_err("create share memory error!");
			return false;
		}
		create = true;
	}

	g_cfgs = (s_config *)shmat(shmid, (void*)SHM_ADDR, 0);
	if (g_cfgs != (void*)-1) {
		if (create) {
			if (!strcmp(g_fn, "")) {
				strcpy(g_fn, SITE_DFT_CONFIG);
			}
			FILE *fp = fopen(g_fn, "r");
			char ln[LEN_LINE];
			char *key, *val;
			int cnt_suc, cnt_fai; cnt_suc = cnt_fai = 0;
			if (fp == NULL) {
				mtc_err("fopen %s failue!\n", g_fn);
				return false;
			}
			shmctl(shmid, SHM_LOCK, NULL);
			while(fgets(ln, LEN_LINE-1, fp) != NULL) {
				if (ln[0] == '#')
					continue;
				key = strtok(ln, CFG_DIVIDER);
				if (key != NULL) {
					val = strtok(NULL, CFG_DIVIDER);
					if (val != NULL) {
						mcfg_setitem(key, val);
						cnt_suc++;
						continue;
					}
				}
				cnt_fai++;
				mtc_warn("'%s' not a legal config", ln);
			}
			shmctl(shmid, SHM_UNLOCK, NULL);
			fclose(fp);
			mtc_noise("load cfgs done, %d ok, %d nok", cnt_suc, cnt_fai);
		}
		return true;
	} else {
		mtc_err("attach share memory failure");
		return false;
	}
}

static bool preconfig()
{
	if (g_cfgs == NULL) {
		if(!cfg_init())
			return false;
	}
	return true;
}

void mcfg_init(const char *fn)
{
	strncpy(g_fn, fn, sizeof(g_fn));
	atexit(mcfg_leave);
}

char* mcfg_getvalue(const char *key, const char *def)
{
	int i;

	if (!preconfig()) {
		return (char*)def;
	}

	for (i = 0; i < g_cfgs->num; i++) {
		if (!strcmp(g_cfgs->key[i], key)) {
			return g_cfgs->value[i];
		}
	}
	return (char*)def;
}
int mcfg_getintvalue(const char *key)
{
	int i;

	if (!preconfig())
		return -1;

	for (i = 0; i < g_cfgs->num; i++) {
		if (!strcmp(g_cfgs->key[i], key)) {
			return atoi(g_cfgs->value[i]);
		}
	}
	return -1;
}

int mcfg_getpos(const char *key)
{
	int i;

	if (!preconfig())
		return -1;

	for (i = 0; i < g_cfgs->num; i++) {
		if (!strcmp(g_cfgs->key[i], key)) {
			return i;
		}
	}
	return -1;
}

bool mcfg_additem(const char *key, const char *value)
{
	if (!preconfig())
		return false;

	int i = g_cfgs->num;
	
	if (i >= CFG_ITEM_NUM)
		return false;
	strncpy(g_cfgs->key[i], key, CFG_ITEM_LEN-1);
	strncpy(g_cfgs->value[i], value, CFG_ITEM_LEN-1);
	g_cfgs->num++;
	
	return true;
}

bool mcfg_setitem(const char *key, const char *value)
{
	if (!preconfig())
		return false;

	int i = mcfg_getpos(key);
	if (i >= 0 && i < CFG_ITEM_NUM) {
		strncpy(g_cfgs->value[i], value, CFG_ITEM_LEN-1);
	} else {
		if (!mcfg_additem(key, value))
			return false;
	}
	return true;
}

void mcfg_leave()
{
	if (g_cfgs != NULL) {
		shmdt((void*)g_cfgs);
		g_cfgs = NULL;
	}
}


NEOERR* mconfig_parse_file(const char *file, HDF **cfg)
{
	NEOERR *err;

	err = hdf_init(cfg);
	if (err != STATUS_OK) return nerr_pass(err);
	
	err = hdf_read_file(*cfg, file);
	if (err != STATUS_OK) return nerr_pass(err);

	return STATUS_OK;
}

void mconfig_cleanup(HDF **config)
{
	if (*config == NULL) return;
	hdf_destroy(config);
}
