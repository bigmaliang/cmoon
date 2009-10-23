
#include <stdio.h>		/* vsprintf() */
#include <stdarg.h>
#include <sys/types.h> 		/* open() */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 		/* write() */
#include <string.h>		/* strcmp(), strerror() */
#include <errno.h>		/* errno */
#include <time.h>		/* time() and friends */

#include "log.h"


/* Logging file descriptor, -1 if logging is disabled */
static char *trace_level[TC_LEVELS] = {"DIE", "MESSAGE", "ERROR", "WARNING", "INFO", "DEBUG", "NOISE"};
static int dftlv = TC_ERROR;
static int logfd = -1;

int log_init(char *logfname)
{
	if (logfname == NULL) {
		logfd = -1;
		return 1;
	}

	if (strcmp(logfname, "-") == 0) {
		logfd = 1;
		return 1;
	}

	logfd = open(logfname, O_WRONLY | O_APPEND | O_CREAT, 0660);
	if (logfd < 0)
		return 0;

	return 1;
}

int log_reopen(char *logfname)
{
	/* Just call log_init(), it will do just fine as we don't need any
	 * special considerations for reopens */
	return log_init(logfname);
}

void log_setlv(int lv)
{
	if (lv < 0 || lv > TC_LEVELS)
		return;
	dftlv = lv;
}

void log_done()
{
	if (logfd != 1 && logfd != -1)
		close(logfd);
}

void wlog(const char *func, const char *file, long line,
		  int level, const char *fmt, ...)
{
	int r, tr, ir;
	va_list ap;
	char str[MAX_LOG_STR];
	char timestr[MAX_LOG_STR];
	char infostr[MAX_LOG_STR];
	time_t t;
	struct tm *tmp;

	if (logfd == -1 || level > dftlv)
		return;

	t = time(NULL);
	tmp = localtime(&t);
	tr = strftime(timestr, MAX_LOG_STR, "[%F %H:%M:%S]", tmp);
	ir = snprintf(infostr, sizeof(infostr), "[%s][%s:%li %s]", trace_level[level], file, line, func);

	va_start(ap, fmt);
	r = vsnprintf(str, MAX_LOG_STR, fmt, ap);
	va_end(ap);

	write(logfd, timestr, tr);
	write(logfd, infostr, ir);
	write(logfd, str, r);
	write(logfd, "\n", 1);
}

