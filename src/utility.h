#ifndef UTILITY_H
#define UTILITY_H

#include <stdbool.h>

#define ERR_UNKNOW_OPT			1
#define ERR_PARSING_INTEGER		2
#define ERR_PARSING_COMMAND		3
#define ERR_UNKNOW_ARGUMENT		4
#define ERR_APPOPT_CONFLICT		5
#define ERR_FILE_INACCESS		6
#define ERR_MISSING_FIELD		7

#define EXIT_PROMPT	1

#define report_error(format, appname, ...) fprintf(stderr, "%s: " format "\n", appname, ##__VA_ARGS__)
#define PRINTABLE_STRING(str) ((str) == NULL ? "(null)" : (str)[0] == '\0' ? "(empty)" : (str))
#define READABLE_BOOLEAN(v) ((v) ? "true" : "false")

bool is_positive_integer(const char *str);

bool is_rw_file(const char *path);

bool is_empty_string(const char *string);

int get_space(char **space, int length);

#endif /* UTILITY_H */