#ifndef __MEVENT_RAWDB_H__
#define __MEVENT_RAWDB_H__

#define PREFIX_RAWDB		"Rawdb"

enum {
	REQ_CMD_ACCESS = 1001,
	REQ_CMD_STAT
} req_cmd_rawdb;

enum {
	REP_OK_RAWDB = 0x65,	/* 101 */
	REP_ERR_RAWDB = 0x401 	/* 1025 */
} rep_code_rawdb;

#endif	/* __MEVENT_RAWDB_H__ */
