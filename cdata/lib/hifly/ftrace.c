#include "fheads.h"

/* global file name for trace info write to */
static char g_fn[LEN_FN] = "";
static FILE *g_fp = NULL;
static char *g_trace_level[TC_LEVELS] = {"DIE", "ERROR", "WARNING", "DEBUG", "INFO", "NOISE"};

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

void ftc_init(const char *fn)
{
	strncpy(g_fn, fn, sizeof(g_fn)-4);
	strcat(g_fn, ".log");
	if (g_fp != NULL)
		FCLOSE(g_fp);
	g_fp = FOPEN(g_fn, "a+");
	atexit(ftc_leave);
}
void ftc_leave()
{
	//ftc_info("close file %s", g_fn);
	if (g_fp != NULL)
		FCLOSE(g_fp);
	g_fp = NULL;
	memset(g_fn, 0x0, sizeof(g_fn));
}

bool ftc_msg(const char *func, const char *file, long line,
			 int level, const char *format, ...)
{
	//int dftlv = fcfg_getintvalue(TC_CFGSTR);
	int dftlv = TC_DEFAULT_LEVEL;
	if (dftlv < 0 || dftlv > TC_LEVELS)
		dftlv = TC_WARNING;
	if (level > dftlv)
		return true;
	
	va_list ap;
	char tm[LEN_TM];
	if (!futil_getdatetime(tm, sizeof(tm), "%F %T", 0))
		return false;

	//FILE *fp;
	//g_fp = fopen(g_fn, "a+");
	if (g_fp == NULL)
		return false;
	FPRINTF(g_fp, "[%s]", tm);
	FPRINTF(g_fp, "[%s]", g_trace_level[level]);
	FPRINTF(g_fp, "[%s:%li %s] ", file, line, func);

	va_start(ap, (void*)format);
	VFPRINTF(g_fp, format, ap);
	va_end(ap);

	FPRINTF(g_fp, "\n");
	//fclose(fp);

	trace_shift_file();
	return true;
}
