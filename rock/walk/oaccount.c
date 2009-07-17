#include "mheads.h"
#include "lheads.h"
#include "omember.h"

#define ACCOUNT_QUERY_COL " uin, uname, status, " \
	" to_char(intime 'YYYY-MM-DD') as intime, to_char(uptime 'YYYY-MM-DD') as uptime "
#define ACCOUNT_QUERY_RAW(conn, condition, sfmt, ...)					\
	mdb_exec(conn, NULL, "SELECT "ACCOUNT_QUERY_COL" FROM accountinfo WHERE %s;", \
			 sfmt, condition, ##__VA_ARGS__)

int account_get_accounts(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	int count, offset, ret;
	
	PRE_DBOP(hdf, conn);

	if (!member_is_root(ses->member)) {
		mtc_err("%d attemped get account, limited", ses->member->uin);
		return RET_RBTOP_LIMITE;
	}

	mmisc_get_offset(hdf, &count, &offset);
	
	ACCOUNT_QUERY_RAW(conn, "1=1 ORDER BY uptime LIMIT $1 OFFSET $2",
					  "ii", count, offset);
	ret = mdb_set_rows(hdf, conn, ACCOUNT_QUERY_COL, PRE_OUTPUT".accounts");

	return ret;
}

int account_add_partner(HDF *hdf, mdb_conn *conn, session_t *ses)
{
	int ret;
	
	PRE_DBOP(hdf, conn);

	if (!member_is_root(ses->member)) {
		mtc_err("%d attemped add account, limited", ses->member->uin);
		return RET_RBTOP_LIMITE;
	}

	int uin, status, rows;
	char *uname;

	uin = hdf_get_int_value(hdf, PRE_QUERY".uin", -1);
	status = hdf_get_int_value(hdf, PRE_QUERY".status", -1);
	uname = hdf_get_value(hdf, PRE_QUERY".uname", NULL);

	if (UIN_ILLEGAL(uin) || uname == NULL || status < 0) {
		return RET_RBTOP_INPUTE;
	}

	ret = member_alloc_user(hdf, conn);
	if (ret == RET_RBTOP_OK) {
		ret = MDATA_SET(conn, EVT_PLUGIN_USER, &rows, FLAGS_NONE,
						"INSERT INTO accountinfo (uin, uname, status) "
						" VALUES (%d, $1, %d)", "s", uin, status, uname);
	} else {
		mtc_err("alloc user failure %d", ret);
	}

	return ret;
}
