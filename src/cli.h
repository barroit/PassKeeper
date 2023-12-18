#ifndef CLI_H
#define CLI_H

void show_version(void);

void show_all_usages(const char *appname);

void show_command_usage(const char *appname, const char *command);

const char *get_create_usage_format(void);

const char *get_read_usage_format(void);

const char *get_update_usage_format(void);

const char *get_delete_usage_format(void);

#endif /* CLI_H */