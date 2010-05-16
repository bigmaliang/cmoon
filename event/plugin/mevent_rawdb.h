#ifndef __MEVENT_RAWDB_H__
#define __MEVENT_RAWDB_H__

#define PREFIX_RAWDB		"Rawdb"

enum {
	REQ_CMD_ACCESS = 1001,
	REQ_CMD_STAT
} req_cmd_rawdb;

enum {
	REP_ERR_RAWDB = 501,
	REP_OK_RAWDB = 1001
} rep_code_rawdb;

#endif	/* __MEVENT_RAWDB_H__ */
