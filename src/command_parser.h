#ifndef COMMAND_PARSER_HEADER
#define COMMAND_PARSER_HEADER

#define OPTION_ALIAS_ID			12160
#define OPTION_ALIAS_SITE_NAME		12161
#define OPTION_ALIAS_SITE_URL		12162
#define OPTION_ALIAS_USERNAME		12163
#define OPTION_ALIAS_PASSWORD		12164
#define OPTION_ALIAS_AUTH_TEXT		12165
#define OPTION_ALIAS_RECOVERY_CODE	12166
#define OPTION_ALIAS_COMMENT		12167

struct argument
{
	char *site_name;
	char *site_url;
	char *username;
	char *password;
	char *auth_text;
	char *recovery_code;
	char *comment;

	char *input_file;

	int id;

	int is_verbose;
	int is_help;

	int subcommand;
};

extern struct argument argument;

void parse_command(int argc, char **argv);

void resolve_option(int c);

int get_subcommand(char **argv);

void resolve_pos_arg(char *value, int position);

int validate_argument(void);

int is_number(char *text);

void print_arguments(void);

#endif