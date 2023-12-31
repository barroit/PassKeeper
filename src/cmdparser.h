#ifndef CMDPARSER_H
#define CMDPARSER_H

#define UNKNOW_OPTION		30
#define UNKNOW_ARGUMENT		31
#define NOT_INTEGER		32
#define COMMAND_MISMATCH	33
#define FIELD_CONFLICT		34
#define FILE_INACCESS		35
#define MISSING_FIELD		36

typedef struct
{
	const char *db_pathname;
	const char *db_key_pathname;

	const char *command;

	const char *sitename;
	const char *siteurl;
	const char *username;
	const char *password;
	const char *authtext;
	const char *bakcode;
	const char *comment;

	int record_id;
	int wrap_threshold;

	int is_help;
	int is_version;
	int is_verbose;
	int is_db_init;

} app_option;

app_option make_appopt(void);

int parse_cmdopts(int argc, char *argv[], app_option *appopt, const char *error_messages[2]);

int handle_parse_option(app_option *appopt, int alias);

int parse_cmdargs(int argc, char *argv[], app_option *appopt);

int handle_parse_command(const char *argument, app_option *appopt);

int handle_parse_argument(const char *argument, app_option *appopt);

int validate_field(char **missing_field, const app_option *appopt);

#endif /* CMDPARSER_H */