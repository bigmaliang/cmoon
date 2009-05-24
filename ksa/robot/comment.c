#include "mheads.h"
#include "lcfg.h"

int comment_get(HDF *hdf, mdb_conn *conn)
{
	PRE_DBOP(hdf, conn);

	char cols[LEN_SM];
	sprintf(cols, " id, pid, zid, sn, sonCnt, intime, uname, content ");
	mdb_exec(conn, NULL, "SELECT %s FROM comment WHERE pid=0 "
			 " ORDER BY id DESC LIMIT 20;", NULL, cols);

	mdb_set_rows(hdf, conn, cols, "comment");

	HDF *res = hdf_get_obj(hdf, PRE_OUTPUT".comment.0");
	while (res != NULL) {
		char *id = hdf_get_value(res, "id", "-1");
		int soncnt = hdf_get_int_value(res, "sonCnt", 0);
		if (soncnt > 0) {
			mdb_exec(conn, NULL, "SELECT %s FROM comment WHERE zid=%s;", NULL, cols, id);
			mdb_set_rows(hdf, conn, cols, "comment");
		}
		res = hdf_obj_next(res);
	}

	return RET_RBTOP_OK;
}

int comment_add(HDF *hdf, mdb_conn *conn)
{
	PRE_DBOP(hdf, conn);

	int id = 0;
	int pid = hdf_get_int_value(hdf, PRE_QUERY".pid", 0);
	int zid = hdf_get_int_value(hdf, PRE_QUERY".zid", 0);
	char *uname = hdf_get_value(hdf, PRE_QUERY".cname", "");
	char *content = hdf_get_value(hdf, PRE_QUERY".ccontent", "");

	mdb_begin(conn);
	/* prepare input */
	int sn = 0;
	if (pid == 0 && zid == 0) {
		mdb_exec(conn, NULL, "SELECT MAX(sn) FROM comment;", NULL, NULL);
		mdb_get(conn, "i", &sn);
		sn += 1;
	}
	char tm[LEN_TM];
	mmisc_getdatetime(tm, sizeof(tm), "%F %T", 0);

	/* process input */
	mdb_exec(conn, NULL, "INSERT INTO comment (pid, zid, sn, sonCnt, intime, uname, content) "
			 " VALUES (%d, %d, %d, 0, '%s', $1, $2);", "ss", pid, zid, sn, tm, uname, content);
	id = mdb_get_last_id(conn, NULL);
	mdb_exec(conn, NULL, "UPDATE comment set sonCnt=sonCnt+1 WHERE id=%d", NULL, pid);
	mdb_exec(conn, NULL, "UPDATE comment set sonCnt=sonCnt+1 WHERE id=%d", NULL, zid);
	mdb_finish(conn);
	if (mdb_get_errcode(conn) != MDB_ERR_NONE) {
		mtc_err("add comment failure. %s", mdb_get_errmsg(conn));
		return RET_RBTOP_INSERTE;
	}

	/* prepare result */
	hdf_set_int_value(hdf, PRE_OUTPUT".comment.0.id", id);
	hdf_set_copy(hdf, PRE_OUTPUT".comment.0.pid", PRE_QUERY".pid");
	hdf_set_copy(hdf, PRE_OUTPUT".comment.0.zid", PRE_QUERY".zid");
	hdf_set_int_value(hdf, PRE_OUTPUT".comment.0.sn", sn);
	hdf_set_value(hdf, PRE_OUTPUT".comment.0.sonCnt","0");
	hdf_set_value(hdf, PRE_OUTPUT".comment.0.intime", tm);
	hdf_set_copy(hdf, PRE_OUTPUT".comment.0.uname", PRE_QUERY".cname");
	hdf_set_copy(hdf, PRE_OUTPUT".comment.0.content", PRE_QUERY".ccontent");
	
	return RET_RBTOP_OK;
}
