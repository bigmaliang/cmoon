#ifndef __MCONFIG_H__
#define __MCONFIG_H__

int config_parse_file(const char *file, HDF **cfg);
void config_cleanup(HDF **config);

#endif    /* __MCONFIG_H__ */
