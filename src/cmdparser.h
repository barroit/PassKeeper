#ifndef CMDPARSER_H
#define CMDPARSER_H

#define CREATE_OPERATION_INDEX	0
#define READ_OPERATION_INDEX	1
#define UPDATE_OPERATION_INDEX	2
#define DELETE_OPERATION_INDEX	3

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

	unsigned record_id;
	unsigned wrap_threshold;

	unsigned command_index;

	int is_help;
	int is_version;
	int is_verbose;
	int is_db_init;

} app_option;

app_option make_appopt(void);

int parse_cmdopts(int argc, char *argv[], app_option *appopt, const char *errmsg[2]);

int handle_parse_option(app_option *appopt, int alias);

int parse_cmdargs(int argc, char *argv[], app_option *appopt, const char *errmsg[3]);

int handle_parse_operation(const char *argument, app_option *appopt);

int handle_parse_argument(const char *argument, app_option *appopt);

int validate_appopt(const app_option *appopt, const char *errmsg[3]);

#endif /* CMDPARSER_H */