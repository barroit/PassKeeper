#ifndef CMDPARSER_H
#define CMDPARSER_H

#define UNKNOW_OPTION		30
#define UNKNOW_ARGUMENT		31
#define NOT_INTEGER		32
#define COMMAND_MISMATCH	33
#define FIELD_CONFLICT		34
#define FILE_INACCESS		35
#define MISSING_FIELD		36

struct app_option
{
	const char *db_filename;

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
};

struct app_option get_appopt(void);

int parse_cmdopts(int argc, char *argv[], struct app_option *appopt);

int parse_cmdargs(int argc, char *argv[], struct app_option *appopt);

int handle_parse_command(const char *argument, struct app_option *appopt);

int handle_parse_argument(const char *argument, struct app_option *appopt);

int validate_field(char **missing_field, const struct app_option *appopt);

#endif /* CMDPARSER_H */