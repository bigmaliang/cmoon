#ifndef __MEVENT_PLACE_H__
#define __MEVENT_PLACE_H__

#define PREFIX_PLACE		"Place"

#define	REDIRECT_MODE_1 	0x01
#define	REDIRECT_MODE_2 	0x02
#define IP_RECORD_LENGTH 	7

enum {
	REQ_CMD_PLACEGET = 1001,
	REQ_CMD_PLACESET
} req_cmd_place;

enum {
	REP_ERR_PLACE = 501,
	REP_OK_PLACE = 1001
} rep_code_place;

#endif	/* __MEVENT_PLACE_H__ */
