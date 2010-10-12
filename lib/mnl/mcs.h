#ifndef __MCS_H__
#define __MCS_H__

#include "mheads.h"

__BEGIN_DECLS

#define DIE_NOK_CGI(err)						\
	if (err != STATUS_OK) {						\
		STRING zstra;	string_init(&zstra);	\
		nerr_error_traceback(err, &zstra);		\
		mtc_err("%s", zstra.buf);				\
		string_clear(&zstra);					\
		cgi_neo_error(cgi, err);				\
		nerr_ignore(&err);						\
		exit(-1);								\
	}

#define JUMP_NOK_CGI(err, label)				\
	if (err != STATUS_OK) {						\
		STRING zstra;	string_init(&zstra);	\
		nerr_error_traceback(err, &zstra);		\
		mtc_err("%s", zstra.buf);				\
		string_clear(&zstra);					\
		cgi_neo_error(cgi, err);				\
		nerr_ignore(&err);						\
		goto label;								\
	}

#define JUMP_NOK(err, label)					\
	if (err != STATUS_OK) {						\
		STRING zstra;	string_init(&zstra);	\
		nerr_error_traceback(err, &zstra);		\
		mtc_err("%s", zstra.buf);				\
		string_clear(&zstra);					\
		nerr_ignore(&err);						\
		goto label;								\
	}

#define DIE_NOK_MTL(err)						\
	if (err != STATUS_OK) {						\
		STRING zstra;	string_init(&zstra);	\
		nerr_error_traceback(err, &zstra);		\
		mtc_err("%s", zstra.buf);				\
		string_clear(&zstra);					\
		nerr_ignore(&err);						\
		exit(-1);								\
	}

#define CONTINUE_NOK(err)						\
	if (err != STATUS_OK) {						\
		STRING zstra;	string_init(&zstra);	\
		nerr_error_traceback(err, &zstra);		\
		mtc_err("%s", zstra.buf);				\
		string_clear(&zstra);					\
		nerr_ignore(&err);						\
		continue;								\
	}

#define RETURN_NOK(err)							\
	if (err != STATUS_OK) {						\
		STRING zstra;	string_init(&zstra);	\
		nerr_error_traceback(err, &zstra);		\
		mtc_err("%s", zstra.buf);				\
		string_clear(&zstra);					\
		nerr_ignore(&err);						\
		return;									\
	}

#define RETURN_V_NOK(err, v)					\
	if (err != STATUS_OK) {						\
		STRING zstra;	string_init(&zstra);	\
		nerr_error_traceback(err, &zstra);		\
		mtc_err("%s", zstra.buf);				\
		string_clear(&zstra);					\
		nerr_ignore(&err);						\
		return v;								\
	}

#define MLIST_ITERATE(list, item)										\
	item = list->items[0];												\
	for (int t_rsv_i = 0; t_rsv_i < list->num; item = list->items[++t_rsv_i])

#define ITERATE_MLIST(ul)                                       \
	for (int t_rsv_i = 0; t_rsv_i < ul->num; t_rsv_i++)


/*
 * 获取请求中参数
 */
#define HDF_GET_INT(hdf, key, ret)				\
    do {										\
		if (!hdf_get_value(hdf, key, NULL)) {	\
            return RET_RBTOP_INPUTE;			\
		}										\
		ret = hdf_get_int_value(hdf, key, 0);	\
    } while (0)

#define HDF_GET_ULONG(hdf, key, ret)							\
    do {														\
		if (!hdf_get_value(hdf, key, NULL)) {					\
            return RET_RBTOP_INPUTE;							\
		}														\
		ret = strtoul(hdf_get_value(hdf, key, NULL), NULL, 10);	\
    } while (0)

#define HDF_GET_STR(hdf, key, ret)				\
    do {										\
		ret = hdf_get_value(hdf, key, NULL);	\
		if (!ret) {								\
            return RET_RBTOP_INPUTE;			\
		}										\
    } while (0)

#define HDF_GET_STR_IDENT(hdf, key, ret)		\
    do {										\
		ret = hdf_get_value(hdf, key, NULL);	\
		if (!ret) {								\
            return RET_RBTOP_NOTLOGIN;			\
		}										\
    } while (0)


#define HDF_FETCH_INT(hdf, key, ret)				\
    do {											\
		if (hdf_get_value(hdf, key, NULL)) {		\
			ret = hdf_get_int_value(hdf, key, 0);	\
		}											\
    } while (0)

#define HDF_FETCH_ULONG(hdf, key, ret)								\
    do {															\
		if (hdf_get_value(hdf, key, NULL)) {						\
			ret = strtoul(hdf_get_value(hdf, key, NULL), NULL, 10); \
		}															\
    } while (0)

#define HDF_FETCH_STR(hdf, key, ret)			\
    do {										\
		ret = hdf_get_value(hdf, key, NULL);	\
    } while (0)


/* because of libneo_cs doesn't have cs_render_stdout */
NEOERR* mcs_outputcb(void *ctx, char *s);
/* because of libneo_cs doesn't have cs_render_to_file */
NEOERR* mcs_strcb(void *ctx, char *s);
bool mcs_str2file(STRING str, const char *file);
void mcs_hdf_escape_val(HDF *hdf);

int mcs_set_login_info(HDF *hdf);

void mcs_rand_string(char *s, int max);

void mcs_text_escape(char *src, char **out);

/*
 * build UPDATE's SET xxxx string
 * data	:IN val {aname: 'foo', pname: 'bar'}
 * node :IN key {0: 'aname', 1: 'pname'}
 * str  :OUT update colum string: aname='foo', pname='bar',
 */
void mcs_build_upcol_s(HDF *data, HDF *node, STRING *str);
void mcs_build_upcol_i(HDF *data, HDF *node, STRING *str);
void mcs_build_querycond_s(HDF *data, HDF *node, STRING *str);
void mcs_build_querycond_i(HDF *data, HDF *node, STRING *str);

__END_DECLS
#endif	/* __MCS_H__ */
