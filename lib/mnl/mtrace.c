#include "mheads.h"

/* global file name for trace info write to */
static char g_fn[LEN_FN] = "";
static FILE *g_fp = NULL;
static char *g_trace_level[TC_LEVELS] = {"DIE", "MESSAGE", "ERROR", "WARNING", "DEBUG", "INFO", "NOISE"};

static void trace_shift_file()
{
	struct stat fs;
	if (stat(g_fn, &fs) == -1)
		return;
	if (fs.st_size < TC_MAX_SIZE)
		return;

	int i;
	char ofn[LEN_FN], nfn[LEN_FN];

	if (g_fp != NULL)
		FCLOSE(g_fp);

	for (i = TC_MAX_NUM-1; i > 1; i--) {
		sprintf(ofn, "%s.%d", g_fn, i-1);
		sprintf(nfn, "%s.%d", g_fn, i);
		rename(ofn, nfn);
	}
	if (TC_MAX_NUM > 1) {
		strcpy(ofn, g_fn);
		sprintf(nfn, "%s.1", g_fn);
		rename(ofn, nfn);
	}

	g_fp = FOPEN(g_fn, "a+");
}

void mtc_init(const char *fn)
{
	strncpy(g_fn, fn, sizeof(g_fn)-4);
	strcat(g_fn, ".log");
	if (g_fp != NULL)
		FCLOSE(g_fp);
	g_fp = FOPEN(g_fn, "a+");
	if (g_fp != NULL) setvbuf(g_fp, (char *)NULL, _IOLBF, 0);
	atexit(mtc_leave);
}
void mtc_leave()
{
	if (g_fp != NULL)
		FCLOSE(g_fp);
	g_fp = NULL;
	memset(g_fn, 0x0, sizeof(g_fn));
}

bool mtc_msg(const char *func, const char *file, long line,
			 int level, const char *format, ...)
{
	int dftlv = hdf_get_int_value(g_cfg, PRE_CONFIG".trace_level", TC_DEFAULT_LEVEL);
	if (level > dftlv)
		return true;
	
	if (g_fp == NULL)
		return false;

	va_list ap;
	char tm[LEN_TM];
	if (!mmisc_getdatetime(tm, sizeof(tm), "%F %T", 0))
		return false;

	FPRINTF(g_fp, "[%s]", tm);
	FPRINTF(g_fp, "[%s]", g_trace_level[level]);
	FPRINTF(g_fp, "[%s:%li %s] ", file, line, func);

	va_start(ap, (void*)format);
	VFPRINTF(g_fp, format, ap);
	va_end(ap);

	FPRINTF(g_fp, "\n");

	trace_shift_file();
	return true;
}

void mcs_build_upcol_s(HDF *data, HDF *node, STRING *str)
{
	if (!data || !node || !str) return;
	
	char *key, *val, *esc;
	
	while (node) {
		key = hdf_obj_value(node);
		val = hdf_get_value(data, key, NULL);
		if (val) {
			mutil_real_escape_string_nalloc(&esc, val, strlen(val));
			string_appendf(str, " %s='%s', ", key, esc);
			free(esc);
		}
		
		node = hdf_obj_next(node);
	}
}

void mcs_build_upcol_i(HDF *data, HDF *node, STRING *str)
{
	if (!data || !node || !str) return;
	
	char *key, *val;
	
	while (node) {
		key = hdf_obj_value(node);
		val = hdf_get_value(data, key, NULL);
		if (val) {
			string_appendf(str, " %s=%d, ", key, atoi(val));
		}
		
		node = hdf_obj_next(node);
	}
}
