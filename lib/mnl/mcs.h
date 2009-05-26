#ifndef __MCS_H__
#define __MCS_H__

#include "mheads.h"

__BEGIN_DECLS

#define DIE_NOK_CGI(err)						\
	if (err != STATUS_OK) {						\
		STRING *zstra;	string_init(zstra);		\
		nerr_error_traceback(err, zstra);		\
		mtc_err("%s", zstra->buf);				\
		string_clear(zstra);					\
		cgi_neo_error(cgi, err);				\
		exit(-1);								\
	}

#define DIE_NOK_MTL(err)						\
	if (err != STATUS_OK) {						\
		STRING *zstra;	string_init(zstra);		\
		nerr_error_traceback(err, zstra);		\
		mtc_err("%s", zstra->buf);				\
		string_clear(zstra);					\
		exit(-1);								\
	}

#define RETURN_NOK(err)							\
	if (err != STATUS_OK) {						\
		STRING *zstra;	string_init(zstra);		\
		nerr_error_traceback(err, zstra);		\
		mtc_err("%s", zstra->buf);				\
		string_clear(zstra);					\
		return;									\
	}

#define RETURN_V_NOK(err, v)					\
	if (err != STATUS_OK) {						\
		STRING *zstra;	string_init(zstra);		\
		nerr_error_traceback(err, zstra);		\
		mtc_err("%s", zstra->buf);				\
		string_clear(zstra);					\
		nerr_ignore(&err);						\
		return v;								\
	}

/* because of libneo_cs doesn't have cs_render_stdout */
NEOERR* mcs_outputcb(void *ctx, char *s);
/* because of libneo_cs doesn't have cs_render_to_file */
NEOERR* mcs_strcb(void *ctx, char *s);
bool mcs_str2file(STRING str, const char *file);

int mcs_set_login_info(HDF *hdf);

__END_DECLS
#endif	/* __MCS_H__ */
