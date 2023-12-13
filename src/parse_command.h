#ifndef PARSE_COMMAND_H
#define PARSE_COMMAND_H

struct app_option
{
	const char *db_file;

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
};

int parse_cmd_opts(int argc, char *argv[], struct app_option *appopt);

int parse_cmd_args(int argc, char *argv[], struct app_option *appopt);

int handle_command_parse(const char *argument, struct app_option *appopt);

int handle_argument_parse(const char *argument, struct app_option *appopt);

int validate_field(char **missing_field, const struct app_option *appopt);

// int preprocess_db_file(struct app_option *appopt);

// int validate_db_file(const char *db_file);

// int validate_command(const char *command);

// int validate_id(const char *id);

// int validate_site_name(const char *site_name);

#endif /* PARSE_COMMAND_H */