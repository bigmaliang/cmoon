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

	/*
	 * prepare src string for strtok. (exactly qrcol string without '(...)')
	 * in : Direction, Actor, CONCAT(Sort1,';',Sort2,';',Sort3,';',Sort4,';',Sort5) AS Sort1
	 * out: Direction, Actor, CONCAT AS Sort1
	 * only support one level '()'
	 */
	memset(src, 0x0, sizeof(src));
	b = qrcol;
	e = strchr(qrcol, '(');
	while (e != NULL) {
		strncat(src, b, e-b);
		b = strchr(e, ')')+1;
		e = strchr(b, '(');
	}
	strncat(src, b, sizeof(src)-1);
	
	p = strtok(src, ",");
	while (p != NULL) {
		//mtc_noise("parse %dst token: '%s'", cnt, p);
		memset(tok,0,sizeof(tok));
		strncpy(tok, p, sizeof(tok)-1);
		b = tok;
		while(*b == '\t' || *b == ' ' || *b == '\r' || *b == '\n') {
			b++;
		}
		e = tok;
		if (strlen(tok) >= 1)
			e += strlen(tok)-1;
		while(*e == '\t' || *e == ' ' || *e == '\r' || *e == '\n') {
			e--;
		}
		strncpy(qr_array[cnt], b, e-b+1);
		//mtc_noise("get tok '%s'", qr_array[cnt]);

		strcpy(tok, qr_array[cnt]);
		bp = strcasestr(tok, " as ");
		if (bp != NULL) {
			//mtc_noise("token '%s' contain ' as '", qr_array[cnt]);
			strncpy(qr_array[cnt], bp+4, sizeof(qr_array[cnt])-1);
			mtc_info("get tok truely '%s'", qr_array[cnt]);
		}
		
		cnt++;
		p = strtok(NULL, ",");
	}
	*qr_cnt = cnt;
}

int mmisc_get_count(mdb_conn *conn, char *table, char *col)
{
	int count = 0;
	mdb_exec(conn, NULL, "SELECT count(*) FROM %s WHERE %s;",
			 NULL, table, col);
	mdb_get(conn, "i", &count);
	return count;
}
void mmisc_set_count(HDF *hdf, mdb_conn *conn, char *table, char *col)
{
	PRE_DBOP_NRET(hdf, conn);
	hdf_set_int_value(hdf, PRE_OUTPUT".ttnum",
					  mmisc_get_count(conn, table, col));
}
void mmisc_get_offset(HDF *hdf, int *count, int *offset)
{
	int i, j;
	i = hdf_get_int_value(hdf, PRE_QUERY".npp", DFT_NUM_PERPAGE);
	j = hdf_get_int_value(hdf, PRE_QUERY".pg", DFT_PAGE_NUM);
	//hdf_set_copy(hdf, PRE_OUTPUT".pg", PRE_QUERY".pg");
	hdf_set_int_value(hdf, PRE_OUTPUT".pg", j);
	*count = i;
	*offset = (j-1)*i;
}

void mmisc_str_repchr(char *s, char from, char to)
{
	if (!s) return;

	while (*s) {
		if (*s == from) *s = to;
		s++;
	}
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
