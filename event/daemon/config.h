#ifndef __MCONFIG_H__
#define __MCONFIG_H__

#define RETURN_V_NOK(err, v)                    \
	if (err != STATUS_OK) {                     \
		STRING zstra;	string_init(&zstra);    \
		nerr_error_traceback(err, &zstra);      \
		wlog("%s", zstra.buf);                  \
		string_clear(&zstra);                   \
		nerr_ignore(&err);                      \
		return v;                               \
	}


int config_parse_file(const char *file, HDF **cfg);
void config_cleanup(HDF **config);

#endif	/* __MCONFIG_H__ */
