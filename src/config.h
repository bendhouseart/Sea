#ifndef CONFIG_H
#define CONFIG_H

typedef struct config
{
    bool parsed;
    int n_levels;
    int n_sources;
    char ** source_mounts;
    char * mount_dir;
    char * log_file;
    int log_level;
    unsigned long max_fs;
    int n_threads;
} config;

char * get_config_file();
struct config get_sea_config();

#endif
