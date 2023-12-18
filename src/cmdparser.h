#ifndef CMDPARSER_H
#define CMDPARSER_H

struct app_option
{
	const char *db_filename;

	const char *command;

	const char *site_name;
	const char *site_url;
	const char *username;
	const char *password;
	const char *auth_text;
	const char *recovery_code;
	const char *comment;

	int record_id;

	int is_help;
	int is_version;
	int is_verbose;
	int is_db_init;
};

struct app_option get_appopt(void);

int parse_cmdopts(int argc, char *argv[], struct app_option *appopt);

int parse_cmdargs(int argc, char *argv[], struct app_option *appopt);

void process_parse_cmdargs_result(int result);

int handle_command_parse(const char *argument, struct app_option *appopt);

int handle_argument_parse(const char *argument, struct app_option *appopt);

int validate_field(char **missing_field, const struct app_option *appopt);

#endif /* CMDPARSER_H */