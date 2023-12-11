#ifndef PROMPT_HEADER
#define PROMPT_HEADER

#define USAGE_ALL		1
#define USAGE_SUBCOMMAND	2

#define ERROR_UNKNOW_COMMAND	1
#define ERROR_UNKNOW_ARGUMENT	2
#define ERROR_MISSING_FIELD	3
#define ERROR_TYPE_MISMATCH	4
#define ERROR_FIELD_CONFLICT	5
#define ERROR_UNDEFINED_ENV	6
#define ERROR_FILE_INACCESSIBLE	7

#define EXIT_HELP		1

#define SAFE_STRING(str) ((str) ? (str) : "(null)")
#define TO_BOOL(v) ((v) ? "true" : "false")

extern char *app_name;
extern char *err_field;
extern char *err_msg;

void version();

void usage(int flag);

void error(int flag);

int strapd(char **str, const char *value);

int strapdf(char **str, const char *format, ...);

int strmgn(char **str, int margin);

#endif