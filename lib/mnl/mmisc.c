#include "mheads.h"

void exiting(void)
{
	mcfg_leave();
}

bool mmisc_getdatetime(char *res, int len, const char *fmt, time_t second)
{
	memset(res, 0x0, len);
	time_t tm = time(NULL) + second;
	struct tm *stm = localtime(&tm);
	if (strftime(res, len, fmt, stm) == 0)
		return false;
	return true;
}

bool mmisc_getdatetime_gmt(char *res, int len, const char *fmt, time_t second)
{
	memset(res, 0x0, len);
	time_t tm = time(NULL) + second;
  	struct tm *stm = gmtime(&tm);
	if (strftime(res, len, fmt, stm) == 0)
		return false;
	return true;
}

int mmisc_compare_int(const void *a, const void *b)
{
	int *i = (int*)a;
	int *j = (int*)b;
	return *i-*j;
}
int mmisc_compare_inta(const void *a, const void *b)
{
	int *i = (int*)a;
	char *j = (char*)b;
	
	return *i - atoi(j);
}

// extract the col NAME request by hdf from SQL statment. e.g. Hash Title Sort1 InsertTime ....
void mmisc_set_qrarray(char *qrcol, char qr_array[QR_NUM_MAX][LEN_ST], int *qr_cnt)
{
	char src[LEN_ML], tok[LEN_ST];

	int cnt = 0;
	char *p;
	char *b, *e, *bp;
	int pos, level;

	/*
	 * prepare src string for strtok. (exactly qrcol string without '(...)')
	 * in : Direction, Actor, CONCAT(Sort1,';',Sort2,';',Sort3,';',Sort4,';',Sort5) AS Sort1, ceil(date_part('epoch', intime)*1000) as   intime
	 * out: Direction, Actor, CONCAT AS Sort1, ceil as    intime
	 */
	memset(src, 0x0, sizeof(src));
	p = qrcol;
	pos = level = 0;
	while (*p && pos < LEN_ML) {
		if (*p != '(' && *p != ')' && level == 0) src[pos++] = *p;
		else {
			if (*p == '(') level++;
			else if (*p == ')') level--;
		}
		p++;
	}
	
	p = strtok(src, ",");
	while (p != NULL) {
		//mtc_noise("parse %dst token: '%s'", cnt, p);
		memset(tok,0,sizeof(tok));
		strncpy(tok, p, sizeof(tok)-1);
		b = tok;
		while(*b && (*b == '\t' || *b == ' ' || *b == '\r' || *b == '\n')) {
			b++;
		}
		e = tok;
		if (strlen(tok) >= 1)
			e += strlen(tok)-1;
		while(*e && (*e == '\t' || *e == ' ' || *e == '\r' || *e == '\n')) {
			e--;
		}
		if (*b == '\0' || *e == '\0') {
			p = strtok(NULL, ",");
			continue;
		}
		strncpy(qr_array[cnt], b, e-b+1);
		//mtc_noise("get tok '%s'", qr_array[cnt]);

		strcpy(tok, qr_array[cnt]);
		bp = strcasestr(tok, " as ");
		if (bp != NULL) {
			//mtc_noise("token '%s' contain ' as '", qr_array[cnt]);
			bp = bp + 4;
			while(*bp && (*bp == '\t' || *bp == ' ' || *bp == '\r' || *bp == '\n')) {
				bp++;
			}
			strncpy(qr_array[cnt], bp, sizeof(qr_array[cnt])-1);
			mtc_info("get tok truely '%s'", qr_array[cnt]);
		}
		
		cnt++;
		p = strtok(NULL, ",");
	}
	*qr_cnt = cnt;
}

void mmisc_pagediv(HDF *hdf, char *inprefix, int *count, int *offset,
				   char *outprefix, HDF *ohdf)
{
	char hdfkey[LEN_HDF_KEY];
	int i, j, npg = DFT_PAGE_NUM;

	if (inprefix) snprintf(hdfkey, sizeof(hdfkey), "%s.npp", inprefix);
	else strcpy(hdfkey, "npp");
	i = hdf_get_int_value(hdf, hdfkey, DFT_NUM_PERPAGE);

	if (outprefix) hdf_set_valuef(ohdf, "%s.npp=%d", outprefix, i);
	else hdf_set_int_value(ohdf, "npp", i);
	
	if (inprefix) snprintf(hdfkey, sizeof(hdfkey), "%s.nst", inprefix);
	else strcpy(hdfkey, "nst");
	j = hdf_get_int_value(hdf, hdfkey, -1);
	if (j == -1) {
		if (inprefix) snprintf(hdfkey, sizeof(hdfkey), "%s.npg", inprefix);
		else strcpy(hdfkey, "npg");
		j = hdf_get_int_value(hdf, hdfkey, DFT_PAGE_NUM);
		npg = j;
		j = (j-1)*i;
	}

	if (outprefix) hdf_set_valuef(ohdf, "%s.npg=%d", outprefix, npg);
	else hdf_set_int_value(ohdf, "npg", npg);
	
	*count = i;
	*offset = j;

	if (outprefix) hdf_set_valuef(ohdf, "%s.nst=%d", outprefix, j);
	else hdf_set_int_value(ohdf, "nst", j);
}

void mmisc_str_repchr(char *s, char from, char to)
{
	if (!s) return;

	while (*s) {
		if (*s == from) *s = to;
		s++;
	}
}

char* mmisc_str_repstr(int rep_count, char *s, ...)
{
	if (!s || rep_count <= 0) return NULL;

	STRING str, tstr;
	char *from, *to, *p, *q;
	size_t len;

	string_init(&str);
	string_init(&tstr);

	string_set(&tstr, s);
	
	va_list ap;
	va_start(ap, s);
	for (int i = 0; i < rep_count; i++) {
		from = va_arg(ap, char*);
		to = va_arg(ap, char*);

		string_clear(&str);
		len = strlen(from);
		q = tstr.buf;
		p = strstr(q, from);
		while (p) {
			string_appendn(&str, q, p-q);
			string_append(&str, to);
			
			q = p + len;
			p = strstr(q, from);
		}
		if (*q) string_append(&str, q);

		string_clear(&tstr);
		string_set(&tstr, str.buf);
	}
	va_end(ap);

	string_clear(&tstr);

	return str.buf;
}

char* mmisc_str_strip (char *s, char n)
{
  int x;

  x = strlen(s) - 1;
  while (x>=0 && s[x]==n) s[x--] = '\0';

  while (*s && *s==n) s++;
  
  return s;
}

unsigned int hash_string(const char *str)
{
        int hash = 5381; // DJB Hash
        const char *s;
	
        for (s = str; *s != '\0'; s++) {
                hash = ((hash << 5) + hash) + tolower(*s);
        }
	
        return (hash & 0x7FFFFFFF);
}


/*
 * IE: make sure timezone & time set correct on web server
 */
void mmisc_cache_headers(time_t second)
{
	/*
	char my_time[256];
	time_t now = time(NULL);
	time_t exp_date = now + second;
	time_t mod_date = now - second;
	*/
		
	//cgiwrap_writef("Cache-Control: public, max-age=%lu\r\n", second);
	cgiwrap_writef("Cache-Control: max-age=%lu\r\n", second);

	/*
	strftime (my_time, 48, "%A, %d-%b-%Y %H:%M:%S GMT",
			  gmtime (&exp_date));
	cgiwrap_writef ("Expires: %s\r\n", my_time);
	*/
		
	//strftime (my_time, 48, "%A, %d-%b-%Y %H:%M:%S GMT",
	//		  gmtime (&mod_date));
	//cgiwrap_writef ("Last-Modified: %s\r\n", my_time);
	//cgiwrap_writef ("Last-Modified: Tue, 02 Jun 2009 05:21:07 GMT\r\n");

	/*
	// Date: should return by web server
	strftime (my_time, 48, "%A, %d-%b-%Y %H:%M:%S GMT",
			  gmtime (&now));
	cgiwrap_writef ("Date: %s\r\n", my_time);
	*/
}

/*
 * use < 10 judgement, or, you can use array ['0', '1', ..., 'e', 'f']
 */
void mmisc_hex2str(unsigned char *hexin, unsigned int inlen, unsigned char *charout)
{
	/* 48 '0' */
	/* 97 'a'  122 'z'  65 'A' */
#define HEX2STR(in, out)						\
	do {										\
		if (((in) & 0xf) < 10) {				\
			(out) = ((in)&0xf) + 48;			\
		} else {								\
			(out) = ((in)&0xf) - 10 + 97;		\
		}										\
	} while (0)

	if (hexin == NULL || charout == NULL)
		return;

	unsigned int i, j;
	memset(charout, 0x0, inlen*2+1);

	for (i = 0, j = 0; i < inlen; i++, j += 2) {
		HEX2STR(hexin[i]>>4, charout[j]);
		HEX2STR(hexin[i], charout[j+1]);
	}

	charout[j+1] = '\0';
}

void mmisc_bin2char(unsigned char *in, unsigned int inlen, unsigned char *out)
{
	/* 48 '0' */
	/* 97 'a'  122 'z'  65 'A' */
#define HEX2STR(in, out)						\
	do {										\
		if (((in) & 0xf) < 10) {				\
			(out) = ((in)&0xf) + 48;			\
		} else {								\
			(out) = ((in)&0xf) - 10 + 97;		\
		}										\
	} while (0)

	if (in == NULL || out == NULL)
		return;

	unsigned int i, j;
	memset(out, 0x0, inlen*2+1);

	for (i = 0, j = 0; i < inlen; i++) {
		if (in[i] == 9 || in[i] == 10 ||
			(in[i] > 31 && in[i] < 127)) {
			/*
			 * resolve printable charactors
			 * see man ascii
			 */
			out[j] = in[i];
			j++;
		} else {
			HEX2STR(in[i]>>4, out[j]);
			HEX2STR(in[i], out[j+1]);
			j += 2;
		}
	}

	out[j+1] = '\0';
}
